using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows.Forms;

namespace TestReception
{
    delegate void UpdateLabel(int ind);
    delegate void UpdateTextBox(string data);

    delegate void DelegateDataReceived(byte[] data);
    delegate bool DelegateStressTest();


    public partial class Form1 : Form
    {
       
        #region field        
        private UpdateLabel _updateLabel;
        private DelegateDataReceived _DelegateDataReceived;
        //private DelegateStressTest _DelegateStressTest;
        private UpdateTextBox _updateTextBox;
        private FileInfo _fichier;
        private Thread _threadStressTest;
        public volatile Mutex _mutexSerial;
        public volatile Mutex _mutexDataUpdate;
        private byte[] _dataBytes;

        private byte[] _serialReadBuffer;

        /// <summary>
        /// delegate that replace SerialPort object DataReceived event
        /// </summary>
        private Action kickoffRead;


        private StreamWriter _sw = null;
        private int _numSend;
        private int _numError;
        private int _receivedTimeOut;
        private int _comp;
        private int _blockLimit;
        private string _path;
        private volatile string _cmd;
        private volatile int _bytesRcved;
        private volatile int _size;
        #endregion

        #region constructor

        public Form1()
        {
            InitializeComponent();

            _mutexSerial = new Mutex();
            _mutexDataUpdate = new Mutex();

            _size = 225792;
            _numSend = 0;
            _numError = 0;
            _comp = 0;
            _cmd = "";
            _path = "";
            // use to dont ask data size at the beginin
            _dataBytes = new byte[225792];
            _receivedTimeOut = 1000;

            _blockLimit = 4096;

            //initialization of received event function
            kickoffRead = null;
            _serialReadBuffer = new byte[_blockLimit];

        }

        #endregion

        #region methods

        /// <summary>
        /// update the combobox with com port
        /// </summary>
        private void LoadPortNames()
        {
            string[] ports;
            ports = SerialPort.GetPortNames();
            this.comboBox_com.Items.Clear();
            foreach (string port in ports)
            {
                comboBox_com.Items.Add(port);
            }
        }
        
        /// <summary>
        /// write data to the recieved texbox
        /// </summary>
        private void Update_Textbox(string data)
        {
            if (InvokeRequired)
            {
                Invoke(_updateTextBox, new Object[] { data });
                return;
            }
            if (data == "")
            {
                string[] lines = this.textBox_reception.Lines;
                lines[0] = "nb error : " + _numError.ToString() + " / " + _numSend.ToString();
                this.textBox_reception.Lines = lines; 
            }
            else
            {
                this.textBox_reception.Text += data + "\r\n";
            }

        }

        /// <summary>
        /// update label with int ind / int data to recieved: "ind" / xxxx
        /// </summary>
        private void Update_Label(int ind)
        {
            if (InvokeRequired)
            {
                Invoke(_updateLabel, new Object[] { ind });
                return;
            }
            this.label_size.Text = ind.ToString() + "/" + _size.ToString();
        }

        /// <summary>
        /// Action to perform when serial data are received
        /// </summary>
        private void DataReceived(byte[] data)
        {
            if (InvokeRequired)
            {
                Invoke(_DelegateDataReceived, new Object[] { data });
                return;
            }

            try
            {
                if (_cmd == "0")
                {
                    lock (_mutexDataUpdate)
                    {
                        data.CopyTo(_dataBytes, _bytesRcved); 

                        if (_bytesRcved >= 4)
                            _size = (int)(_dataBytes[0] << 24) + (int)(_dataBytes[1] << 16) + (int)(_dataBytes[2] << 8) + (int)(_dataBytes[3]);

                        _bytesRcved += data.Length;
                    }
                    _updateTextBox("size : " + _size.ToString());
                    _updateLabel(0);
                }
                else
                {
                    lock (_mutexDataUpdate)
                    {
                        data.CopyTo(_dataBytes, _bytesRcved);
                        _bytesRcved += data.Length;
                        _updateLabel(_bytesRcved); 
                    }
                }
            }
            catch (Exception e)
            {
                if (serialPort1.IsOpen)
                {
                    this.serialPort1.ReadExisting();
                    lock (_mutexDataUpdate)
                    {
                        _dataBytes.Initialize(); 
                    }
                }
                //throw e;
            }

        }

        /// <summary>
        /// creat a new file data.txt on the apllication folder
        /// </summary>
        private void NewFile()
        {
            _path = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + @"\data.txt";
            _fichier = new FileInfo(_path);
            try
            {
                if (_fichier.Exists)
                    _fichier.Delete();
                _fichier.Create();
            }
            catch (Exception e)
            {
                MessageBox.Show("error : " + e.Message, "file creation issue");
            }
        }

        /// <summary>
        /// creat string array of 256 bytes(512 char) and test correspondance
        /// </summary>
        private string[] FormatingData(byte[] bytes)
        {
            int size = 512;
            int mod = bytes.Length % size;
            int div = bytes.Length / size;
            int lineCount = bytes.Length / 256;
            int ind = 0;
            string[] data;
            Regex reg = new Regex("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F404142434445464748494A4B4C4D4E4F505152535455565758595A5B5C5D5E5F606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E7F808182838485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9FA0A1A2A3A4A5A6A7A8A9AAABACADAEAFB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBFC0C1C2C3C4C5C6C7C8C9CACBCCCDCECFD0D1D2D3D4D5D6D7D8D9DADBDCDDDEDFE0E1E2E3E4E5E6E7E8E9EAEBECEDEEEFF0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF");

            string byte2string = BitConverter.ToString(bytes);
            byte2string = byte2string.Replace("-", "");
            int matches = reg.Matches(byte2string).Count;
            //1764
            if (matches != lineCount)
                _numError += 1;

            Update_Textbox("");

            if (mod != 0)
            {
                data = new string[div + 1];
            }                
            else
            {
                data = new string[div];
            }

            ind = 0;
            for(int i =0; i < div; i++)
            {
                data[i] = BitConverter.ToString(bytes, ind, size).Replace("-", "");
                ind += size;
            }

            if (mod != 0)
            {
                data[div] = byte2string.Substring(byte2string.Length - mod , mod);
            }

            return data;
        }

        /// <summary>
        /// write data to the .txt file
        /// </summary>
        private void WriteText(string[] data)
        {
            try
            {
                if (_fichier.Exists )
                {
                    _sw = new StreamWriter(_path, false, System.Text.Encoding.ASCII);
                    
                    int size = data.Length;
                    for (int i = 0; i < size; i++)
                    {
                        _sw.WriteLine(data[i]);
                    }
                    _sw.Close();
                }
                else
                {
                    NewFile();
                    throw new Exception("file doesn't exist");
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("error : " + e.Message, "report issue");
            }
        }

        /// <summary>
        /// Stress test
        /// </summary>
        private bool StressTest()
        {
            /*if (InvokeRequired)
            {
                bool res = (bool)Invoke(_DelegateStressTest, new Object[] {  });
                return res;
            }*/

            bool errorNotDetetcted = true;
            int endTime = Environment.TickCount + _receivedTimeOut;


            while (errorNotDetetcted)
            {
                lock (_mutexSerial)
                {
                    this.serialPort1.DtrEnable = true;
                    this.serialPort1.Write(_cmd);
                    _bytesRcved = 0;
                    _numSend++;
                }

                while (_bytesRcved < _size && Environment.TickCount < endTime) ;

                lock (_mutexSerial)
                {
                    string[] data = FormatingData(_dataBytes);
                    if (_numError != _comp)
                    {
                        _comp = _numError;
                        WriteText(data);
                        errorNotDetetcted = false;
                    }
                    //Thread.Sleep(10);
                } 
            }

            return errorNotDetetcted;
        }

        /// <summary>
        /// Perfor cyclic writing/reading until an error occur
        /// </summary>
        private void PerformStressTestTh()
        {
            if(_threadStressTest == null)
            {
                _threadStressTest = new Thread(() =>
                {
                    StressTest();
                    _threadStressTest = null;
                });
                _threadStressTest.Start();
            }
            else
            {
                _threadStressTest.Abort();
                _threadStressTest = null;
            }
        }

        #endregion

        #region event handler

        private void Form1_Load(object sender, EventArgs e)
        {
            //delegate to update from from data recieved method
            _updateLabel = new UpdateLabel(Update_Label);
            _updateTextBox = new UpdateTextBox(Update_Textbox);
            _DelegateDataReceived = new DelegateDataReceived(DataReceived);
            _threadStressTest = null;
            //_DelegateStressTest = new DelegateStressTest(StressTest);

            kickoffRead = delegate
            {
                try
                {
                    this.serialPort1.BaseStream.BeginRead(_serialReadBuffer, 0, _serialReadBuffer.Length, delegate (IAsyncResult ar)
                   {
                       try
                       {
                           if (serialPort1.IsOpen)
                           {

                               int actualLength = serialPort1.BaseStream.EndRead(ar);
                               byte[] received = new byte[actualLength];
                               Buffer.BlockCopy(_serialReadBuffer, 0, received, 0, actualLength);
                               if (actualLength > 0)
                                   DataReceived(received);

                           }
                           kickoffRead();
                       }
                       catch (Exception ie )//ie for internql exceeption
                       {
                           MessageBox.Show(ie.Message, "Serial issue");
                       }
                   }, null);
                }
                catch (IOException exc)
                {
                    if (serialPort1.IsOpen)
                        serialPort1.Close();
                }
            };

            NewFile();
        }

        private void button_connexion_Click(object sender, EventArgs e)
        {
            Regex COM_PORT = new Regex( "COM[1-9]{1,3}");

            if (this.serialPort1.IsOpen || this.button_connexion.Text == "disconnect")
            {
                this.serialPort1.Close();
                this.button_connexion.Text = "connect";
                this.flowLayoutPanel1.Enabled = false;
            }
            else if (COM_PORT.Match(this.comboBox_com.Text).Success)
            {
                //this.serialPort1 = new SerialPort(this.comboBox1.Text);
                this.serialPort1.PortName = this.comboBox_com.Text;
                this.serialPort1.BaudRate = 115200;
                //this.serialPort1.BaudRate = 9600;
                this.serialPort1.Parity = Parity.None;
                this.serialPort1.DataBits = 8;
                //this.serialPort1.ReadBufferSize = 64;
                this.serialPort1.ReadTimeout = 250;
                this.serialPort1.Handshake = Handshake.XOnXOff;
                try
                {
                    this.serialPort1.Open();
                    kickoffRead();
                    this.button_connexion.Text = "disconnect";
                    this.flowLayoutPanel1.Enabled = true;
                }
                catch (Exception ex)
                {
                    MessageBox.Show("can't open com por: "+ex.Message);
                }
                
            }
            _bytesRcved = 0;
            
        }

        private void button_sendCmd_Click(object sender, EventArgs e)
        {
            
            _cmd = this.textBox_CMD.Text;
            //_lBytes.Clear();
            try
            {
                lock (_mutexSerial)
                {
                    if (this.serialPort1.IsOpen)
                    {
                        //_dataBytes.Initialize();
                        this.serialPort1.DtrEnable = true;
                        //this.serialPort1.RtsEnable = false ; 
                        this.serialPort1.Write(_cmd);
                        //this.serialPort1.DtrEnable = false;
                        //this.serialPort1.RtsEnable = true;
                        if (_cmd == "1")
                        {
                            _bytesRcved = 0;
                            //this.serialPort1.Write(cmd);
                            _numSend++;
                            this.button_textWrite.Select();
                        }

                    }
                    else
                    {
                        this.textBox_reception.Text = "DISCONNECTED";
                        if (this.button_connexion.Text == "disconnect")
                            this.button_connexion.PerformClick();

                    } 
                } 
            }
            catch (Exception ex)
            {
                MessageBox.Show("error : " + ex.Message, "COM port issue");
            }
        }
        
        /*private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            //this.serialPort1.DtrEnable = false;
            const int nReadBlockSize = 32;

            int size = this.serialPort1.BytesToRead;
            int nCurrentReadSize;

            //test if an uint32 is recieved
            if(_cmd == "0")
            {
                byte[] data = { 0, 0, 0, 0 };
                this.serialPort1.Read(data, 0, 4);
                _size = (int)(data[0] <<24) + (int)(data[1] <<16)  + (int)(data[2] << 8)  + (int)(data[3]) ;
                _dataBytes = new byte[_size];
                this.serialPort1.DiscardInBuffer();
                _updateTextBox("size : " + _size.ToString());
                _updateLabel(0);
            }
            else //else add data to _lBytes
            {
                //byte[] buffer = new byte[size];
                //this.serialPort1.Read(buffer, 0, size);
                //this.serialPort1.Read(_dataBytes, _bytesRcved, size);

                do
                {
                    if (size > nReadBlockSize)
                    {
                        nCurrentReadSize = nReadBlockSize;
                    }
                    else
                    {
                        nCurrentReadSize = size;
                    }

                    this.serialPort1.Read(_dataBytes, _bytesRcved, nCurrentReadSize);

                    size -= nCurrentReadSize;
                    _bytesRcved += nCurrentReadSize;

                } while (size != 0);
                
                //for (int i =0; i < size; i++)
                //{
                //    _dataBytes[i + _bytesRcved] = (byte)serialPort1.
                //}/
                
                //{
                //dummy
                // if there is a particular pattern "000000" for example, we manually try to stop the transfer
                // need to explore if there is a option for the host to set into " not ready " state
                // in order to stop the transfer from the device
                //}

                //_lBytes.AddRange(new List<byte>(buffer));
                //_updateLabel(_lBytes.Count);
                //_bytesRcved += size;
                _updateLabel(_bytesRcved);
            }

            //this.serialPort1.DtrEnable = true;
        }*/

        private void comboBox_com_MouseClick(object sender, MouseEventArgs e)
        {
            LoadPortNames();
        }
       
        private void button_textWrite_Click(object sender, EventArgs e)
        {
            //for (int i = 0; i < 100; i++)
            //while(_size > _bytesRcved)
            //{
            //    if (this.serialPort1.BytesToRead != 0)
            //    {
            //        this.serialPort1.DtrEnable= false;
            //        int size = this.serialPort1.BytesToRead;
            //        this.serialPort1.Read(_dataBytes, _bytesRcved, size);
            //        this.serialPort1.DtrEnable = true;
            //        _bytesRcved += size;
            //        _updateLabel(_bytesRcved);
            //    }
            //    //Thread.Sleep(10);
            //}
            //this.serialPort1.DiscardInBuffer();
            string[] data = new string[0];
            lock (_mutexDataUpdate)
            {
                data = FormatingData(_dataBytes); 
            }
            if(_numError != _comp)
            {

                _comp = _numError;
                WriteText(data);
            }
            //for (int i = 0; i < _dataBytes.Length; i++)
            //    _dataBytes[i] = 0x00;
            this.button_sendCmd.Select();
        }

        #endregion

        private void button_stressTest_Click(object sender, EventArgs e)
        {
            if (_size == 0)
                return;
            else if (_cmd != "1")
                _cmd = "1";

            PerformStressTestTh();

        }
    }
}