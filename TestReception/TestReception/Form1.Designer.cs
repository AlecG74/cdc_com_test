namespace TestReception
{
    partial class Form1
    {
        /// <summary>
        /// Variable nécessaire au concepteur.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Nettoyage des ressources utilisées.
        /// </summary>
        /// <param name="disposing">true si les ressources managées doivent être supprimées ; sinon, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Code généré par le Concepteur Windows Form

        /// <summary>
        /// Méthode requise pour la prise en charge du concepteur - ne modifiez pas
        /// le contenu de cette méthode avec l'éditeur de code.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.comboBox_com = new System.Windows.Forms.ComboBox();
            this.button_connexion = new System.Windows.Forms.Button();
            this.serialPort1 = new System.IO.Ports.SerialPort(this.components);
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.textBox_reception = new System.Windows.Forms.TextBox();
            this.textBox_CMD = new System.Windows.Forms.TextBox();
            this.button_sendCmd = new System.Windows.Forms.Button();
            this.label_size = new System.Windows.Forms.Label();
            this.button_textWrite = new System.Windows.Forms.Button();
            this.button_stressTest = new System.Windows.Forms.Button();
            this.flowLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // comboBox_com
            // 
            this.comboBox_com.FormattingEnabled = true;
            this.comboBox_com.Location = new System.Drawing.Point(27, 13);
            this.comboBox_com.Name = "comboBox_com";
            this.comboBox_com.Size = new System.Drawing.Size(121, 24);
            this.comboBox_com.TabIndex = 0;
            this.comboBox_com.Text = "COM13";
            this.comboBox_com.MouseClick += new System.Windows.Forms.MouseEventHandler(this.comboBox_com_MouseClick);
            // 
            // button_connexion
            // 
            this.button_connexion.Location = new System.Drawing.Point(27, 44);
            this.button_connexion.Name = "button_connexion";
            this.button_connexion.Size = new System.Drawing.Size(121, 23);
            this.button_connexion.TabIndex = 1;
            this.button_connexion.Text = "connexion";
            this.button_connexion.UseVisualStyleBackColor = true;
            this.button_connexion.Click += new System.EventHandler(this.button_connexion_Click);
            // 
            // serialPort1
            // 
            this.serialPort1.BaudRate = 15200;
            this.serialPort1.WriteTimeout = 500;
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.Controls.Add(this.textBox_reception);
            this.flowLayoutPanel1.Controls.Add(this.textBox_CMD);
            this.flowLayoutPanel1.Controls.Add(this.button_sendCmd);
            this.flowLayoutPanel1.Controls.Add(this.label_size);
            this.flowLayoutPanel1.Controls.Add(this.button_textWrite);
            this.flowLayoutPanel1.Controls.Add(this.button_stressTest);
            this.flowLayoutPanel1.Location = new System.Drawing.Point(166, 12);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(299, 255);
            this.flowLayoutPanel1.TabIndex = 2;
            // 
            // textBox_reception
            // 
            this.textBox_reception.Location = new System.Drawing.Point(3, 3);
            this.textBox_reception.Multiline = true;
            this.textBox_reception.Name = "textBox_reception";
            this.textBox_reception.ReadOnly = true;
            this.textBox_reception.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox_reception.Size = new System.Drawing.Size(279, 155);
            this.textBox_reception.TabIndex = 0;
            // 
            // textBox_CMD
            // 
            this.textBox_CMD.Location = new System.Drawing.Point(3, 164);
            this.textBox_CMD.Name = "textBox_CMD";
            this.textBox_CMD.Size = new System.Drawing.Size(191, 22);
            this.textBox_CMD.TabIndex = 1;
            // 
            // button_sendCmd
            // 
            this.button_sendCmd.Location = new System.Drawing.Point(200, 164);
            this.button_sendCmd.Name = "button_sendCmd";
            this.button_sendCmd.Size = new System.Drawing.Size(75, 23);
            this.button_sendCmd.TabIndex = 2;
            this.button_sendCmd.Text = "send";
            this.button_sendCmd.UseVisualStyleBackColor = true;
            this.button_sendCmd.Click += new System.EventHandler(this.button_sendCmd_Click);
            // 
            // label_size
            // 
            this.label_size.Location = new System.Drawing.Point(3, 190);
            this.label_size.Name = "label_size";
            this.label_size.Size = new System.Drawing.Size(109, 15);
            this.label_size.TabIndex = 3;
            this.label_size.Text = "../..";
            this.label_size.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // button_textWrite
            // 
            this.button_textWrite.Location = new System.Drawing.Point(118, 193);
            this.button_textWrite.Name = "button_textWrite";
            this.button_textWrite.Size = new System.Drawing.Size(75, 23);
            this.button_textWrite.TabIndex = 4;
            this.button_textWrite.Text = "write text";
            this.button_textWrite.UseVisualStyleBackColor = true;
            this.button_textWrite.Click += new System.EventHandler(this.button_textWrite_Click);
            // 
            // button_stressTest
            // 
            this.button_stressTest.Location = new System.Drawing.Point(3, 222);
            this.button_stressTest.Name = "button_stressTest";
            this.button_stressTest.Size = new System.Drawing.Size(179, 23);
            this.button_stressTest.TabIndex = 5;
            this.button_stressTest.Text = "Stress test";
            this.button_stressTest.UseVisualStyleBackColor = true;
            this.button_stressTest.Click += new System.EventHandler(this.button_stressTest_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(475, 279);
            this.Controls.Add(this.flowLayoutPanel1);
            this.Controls.Add(this.button_connexion);
            this.Controls.Add(this.comboBox_com);
            this.Name = "Form1";
            this.Text = "Scanner Test Reception";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBox_com;
        private System.Windows.Forms.Button button_connexion;
        private System.IO.Ports.SerialPort serialPort1;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.TextBox textBox_reception;
        private System.Windows.Forms.TextBox textBox_CMD;
        private System.Windows.Forms.Button button_sendCmd;
        private System.Windows.Forms.Label label_size;
        private System.Windows.Forms.Button button_textWrite;
        private System.Windows.Forms.Button button_stressTest;
    }
}

