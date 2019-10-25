﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Net;
using System.Net.Sockets;

using System.Diagnostics;

using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Exceptions;
using uPLibrary.Networking.M2Mqtt.Internal;
using uPLibrary.Networking.M2Mqtt.Messages;
using uPLibrary.Networking.M2Mqtt.Session;
using uPLibrary.Networking.M2Mqtt.Utility;

using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Net.Security;

using Newtonsoft.Json;

using System.IO;
//using System.Json;
using PeterO;
using PeterO.Cbor;
using PeterO.Numbers;




namespace MqttClientSimulatorBinary
{

    public partial class Form1 : Form
    {
        const string JSON_VALIDATOR = "https://jsonformatter.curiousconcept.com/";
        // "https://jsonformatter.org/";    
        // "https://jsonformatter.curiousconcept.com/"

        const string MQTT_BROKER_ADDRESS = "192.168.16.102";

        const string SUB_TEMP  = "alessandro_bilato/f/temperature";
        const string SUB_RELE1 = "alessandro_bilato/f/rele1";
        const string PUB_RELE1 = SUB_RELE1;

        const string SUB_TEMP_carel = "sonda";
        const string PUB_VAL_carel  = "pco/topic/valore";


        const string VAL_REQ_carel  = @"/req";
        public string val_req_post;

        //const string VAL_RESP_carel = @"/res/53BCE4F1DFA0FE8E7CA126F91B35D3A6";

        public string VAL_RESP_carel = @"/res/53BCE4F1DFA0FE8E7CA126F91B35D3A6";
        public string VAL_RESP_carel_ALL = @"#";

        public string VAL_TOPIC_CONNECTED = @"";
        public string VAL_TOPIC_STATUS = @"";


        //static int value = 0;

        public string server_url;
        public int server_port;

        MqttClient client;
        public bool MQTT_Connect;

        public double start_time, stop_time;

        public double start_time_res2res;
        public bool start_stop_res2res_flag = false;

        string Myjson = null;

        public int msg_line_count = 1;


        private void MessageBoxInfo(string message, string caption)
        {
            // Checks the value of the text.
            if (checkBox_ShowMsg.Checked == true)
            {
                // Initializes the variables to pass to the MessageBox.Show method.
                MessageBoxButtons buttons = MessageBoxButtons.OK; //MessageBoxButtons.YesNo;
                DialogResult result;

                // Displays the MessageBox.
                result = MessageBox.Show(message, caption, buttons);
                if (result == System.Windows.Forms.DialogResult.Yes)
                {

                }
            }
        }
        
        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (MQTT_Connect == true)
            {
                client.Disconnect();
            }
        }

        public Form1()
        {
            InitializeComponent();

            //Load settings






            FormClosed += Form1_FormClosed;

            //System.Net.IPAddress IPAddress = System.Net.IPAddress.Parse(MQTT_BROKER_ADDRESS);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Debug.WriteLine("MessageId 1");
        }


        private void Timer_Pub2Res(bool start_stop)
        {

            string nstr;

            if (start_stop == false)
            {
                start_time = (new TimeSpan(DateTime.Now.Ticks)).TotalMilliseconds;
                nstr = @"-----";
                txtConsole.Invoke(new Action(() => textBox_timer_pub2res.Text = nstr));
            }
            else
            {
                stop_time = (new TimeSpan(DateTime.Now.Ticks)).TotalMilliseconds;
                double totalMill = stop_time - start_time;
            

                try
                {                    
                    nstr = totalMill.ToString();
                    txtConsole.Invoke(new Action(() => textBox_timer_pub2res.Text=nstr));                 
                }
                catch (System.InvalidOperationException)
                {
                    //Debug.WriteLine("-->ERROR ERROR ERROR");
                    MessageBoxUpdated("Timer_Pub2Res Error!");
                }

            }

        }




        private void Timer_Res2Res(bool start_stop)
        {

            string nstr;
            

            if (start_stop_res2res_flag == false)
            {
                start_time_res2res = (new TimeSpan(DateTime.Now.Ticks)).TotalMilliseconds;
                nstr = @"-----";
                txtConsole.Invoke(new Action(() => textBox_Res2Res.Text = nstr));
                start_stop_res2res_flag = true;
            }
            else
            {
                stop_time = (new TimeSpan(DateTime.Now.Ticks)).TotalMilliseconds;
                double totalMill = stop_time - start_time_res2res;

                //for the next iteration
                start_time_res2res = stop_time;


                try
                {
                    nstr = totalMill.ToString();
                    txtConsole.Invoke(new Action(() => textBox_Res2Res.Text = nstr));
                }
                catch (System.InvalidOperationException)
                {
                    //Debug.WriteLine("-->ERROR ERROR ERROR");
                    MessageBoxUpdated("Timer_Res2Res Error!");
                }

            }

        }





        private void MessageBoxUpdated(string msg)
        {
            string s_msg;

            s_msg = Environment.NewLine + (new TimeSpan(DateTime.Now.Ticks)).TotalMilliseconds + @" - " + msg_line_count.ToString() + @" " + msg ;
            textBox_Message.Invoke(new Action(() => textBox_Message.AppendText(s_msg)));
            msg_line_count += 1;
        }

               


        void client_MqttMsgPublished(object sender, MqttMsgPublishedEventArgs e)
        {
            //Debug.WriteLine("MessageId = " + e.MessageId + " Published = " + e.IsPublished);

            MessageBoxUpdated("MessageId = " + e.MessageId + " Published = " + e.IsPublished);
            Timer_Pub2Res(false);
        }


        void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            string payload = null;

            Timer_Pub2Res(true);
            Timer_Res2Res(true);


            bool targer_topic_for_me = false;
            targer_topic_for_me = e.Topic.Contains(textBox_Target.Text);


            //if ((e.Topic == VAL_TOPIC_CONNECTED) || (e.Topic == VAL_TOPIC_STATUS))
            //{
            //    payload = Encoding.UTF8.GetString(e.Message);
            //    txtConsole.Invoke(new Action(() => txtConsole.AppendText(payload + Environment.NewLine)));
            //}
            CBORObject cbor_rx;
            cbor_rx = CBORObject.DecodeFromBytes(e.Message);





            //BILATO receive the CBOR payload


            if ((targer_topic_for_me == true))
            {
                //payload = Encoding.UTF8.GetString(e.Message);
                //txtConsole.Invoke(new Action(() => txtConsole.AppendText(payload + Environment.NewLine)));
                txtConsole.Invoke(new Action(() => txtConsole.AppendText(cbor_rx.ToString())));



            }
            else
            {
                //default on # box
                //payload = Encoding.UTF8.GetString(e.Message);                                           
                //txtConsole.Invoke(new Action(() => textBox_Resp_Hash.AppendText(payload + Environment.NewLine)));
                txtConsole.Invoke(new Action(() => textBox_Resp_Hash.AppendText(cbor_rx.ToString())));
            }


            MessageBoxUpdated(" e.Topic = " + e.Topic);

        }




        // call back for connection SSL
        bool RemoteCertificateValidationCallback(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            // logic for validation here
            return true;
        }


        //*    json     *//

        public class Account
        {
            public string Email { get; set; }
            public bool Active { get; set; }
            public DateTime CreatedDate { get; set; }
            public IList<string> Roles { get; set; }
        }


        private void PublishTestFile(string testfilepath)
        {
            string textFile = null;
            byte[] fileBytes;

            if (File.Exists(testfilepath))
            {
                 // Read entire text file content in one string  
                 //textFile = File.ReadAllText(testfilepath);                
                 fileBytes = File.ReadAllBytes(testfilepath);
            }
            else
            {
                textBoxPublish.Text = "File not found " + testfilepath;
                return;
            }

            try
            {
                ushort msgId = client.Publish(val_req_post, fileBytes, MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);

                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                //Debug.WriteLine("Not Initialized");
                MessageBoxUpdated("MQTT not initialized!");
            }



        }

        void test_json()
        {

            Account account = new Account
            {
                Email = "carel@example.com",
                Active = true,
                CreatedDate = new DateTime(2019, 1, 20, 0, 0, 0, DateTimeKind.Utc),
                Roles = new List<string>
                {
                "User",
                "Admin"
                }
            };

            Myjson = JsonConvert.SerializeObject(account, Formatting.Indented);

            //Debug.WriteLine(Myjson);

            MessageBoxUpdated(Myjson);
        }

        private void ButtonChangeCredential_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_CHANGE_CREDENTIALS.cbor";    
            
            PublishTestFile(textFilePath);
            Timer_Pub2Res(false);
        }

        private void ButtonReadValue_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_READ_VALUES.cbor";
            PublishTestFile(textFilePath);
        }

        private void ButtonClear_Click(object sender, EventArgs e)
        {
            textBoxPublish.Text = "";
        }

        private void ButtonScanLine_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_SCAN_DEVICES.cbor";
            PublishTestFile(textFilePath);
        }

        private void ButtonSet_Lines_Config_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_SET_LINES_CONFIG.cbor";
            PublishTestFile(textFilePath);
        }

        private void buttonSet_Devs_Config_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_DOWNLOAD_DEVS_CONFIG.cbor";
            PublishTestFile(textFilePath);
        }

        private void ButtonUpdate_ca_cerficates_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_UPDATE_CA.cbor";
            PublishTestFile(textFilePath);
        }

        private void ButtonUpdate_dev_firmware_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\REQ_UPDATE_DEV_FIRMWARE.cbor";
            PublishTestFile(textFilePath);
        }

        private void ButtonUpdate_gw_firmware_Click(object sender, EventArgs e)
        {
            string textFilePath = @".\cbor_cloud\test-update_gw_firmware-req.json";
            PublishTestFile(textFilePath);
        }







        private void ButtonConnect_Click(object sender, EventArgs e)
        {

            if (MQTT_Connect == true)
            {
                client.Disconnect();
                MQTT_Connect = false;
                buttonConnect.Text = "Connect";
                buttonConnect.BackColor = Color.Green;
                textBox_SubTopic.Enabled = true;
                textBox_Target.Enabled = true;
                return;
            }


            VAL_RESP_carel      = textBox_Target.Text + @"/" +  textBox_SubTopic.Text;
            VAL_TOPIC_CONNECTED = textBox_Target.Text + @"/connected";
            VAL_TOPIC_STATUS    = textBox_Target.Text + @"/status";


            val_req_post = textBox_Target.Text + VAL_REQ_carel;

            textBox_SubTopic.Enabled = false;
            textBox_Target.Enabled = false;
                                          


            // create client instance 
            MessageBoxUpdated("--> create client instance ");
            
            //client = new MqttClient("io.adafruit.com");        //(IPAddress.Parse("MQTT_BROKER_ADDRESS"));

            // import certificate   (step 1)
            string path = Directory.GetCurrentDirectory() + "\\cert\\ca.crt";
            X509Certificate2 caCert = new X509Certificate2(path);

            this.server_url = textBoxMQTT_Server_URL.Text;
            
            this.server_port = Int32.Parse(textBoxMQTT_Server_Port.Text);

            // new for Carel broker  (step 2) 
            server_url = textBoxMQTT_Server_URL.Text;

            if (checkBox_TLS.Checked == true)
            {
                client = new MqttClient(server_url, server_port, true, caCert, caCert, MqttSslProtocols.TLSv1_2, RemoteCertificateValidationCallback);
            }
            else
            {
                client = new MqttClient(server_url);

            }

            // register to message received 
            client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;
            client.MqttMsgPublished += client_MqttMsgPublished;

            string clientId = Guid.NewGuid().ToString();

            //client.Connect(clientId);                                                              // for local Mosquitto
            //client.Connect(clientId, "alessandro_bilato", "51ed38a4a4d14de09f021ee0de2db993");     // for Iot Adafruit    
            client.Connect(clientId, "admin", "5Qz*(3_>K&vU!PS^");

            if (client.IsConnected)
            {
                MessageBoxUpdated("--> client CONNECT ");
            }
            else
            {
                MessageBoxUpdated("--> client NOT CONNECT ");
            }

            // subscribe to the topic "/home/temperature" with QoS 2 
            MessageBoxUpdated("--> subscribe to the topic " + VAL_RESP_carel);

            //client.Subscribe(new string[] { SUB_TEMP }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });
            //client.Subscribe(new string[] { SUB_RELE1 }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });

            //client.Subscribe(new string[] { SUB_TEMP_carel }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });
            //client.Subscribe(new string[] { PUB_VAL_carel }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });


            client.Subscribe(new string[] { VAL_RESP_carel_ALL }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE });

            MQTT_Connect = true;
            buttonConnect.Text = "Disconnect";
            buttonConnect.BackColor = Color.Red; 
        }

        private void TextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void ButtonClearResponse_Click(object sender, EventArgs e)
        {
            txtConsole.Text = "";
        }

        private void Label4_Click(object sender, EventArgs e)
        {

        }

        private void TextBox1_TextChanged_1(object sender, EventArgs e)
        {

        }






    


        private void Button_MB_Read_HR_Click(object sender, EventArgs e)
        {                                
            const string template_name = @"test-read_values_hr_03.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }
            
            string vAlias = textBox_Alias.Text;
            textFile = textFile.Replace("$$ALIAS", vAlias);

            //string vVal = textBox_HR_Val.Text;
            //textFile = textFile.Replace("$$VAL", vVal);

            string vFunc = textBox_MB_HR_R_Func.Text;
            textFile = textFile.Replace("$$FUNC", vFunc);

            string vAddr = textBox_MB_Addr.Text;
            textFile = textFile.Replace("$$ADDR", vAddr);

            string vDim = textBox_MB_Dim.Text;
            textFile = textFile.Replace("$$DIM", vDim);

            string vPos = textBox_MB_Pos.Text;
            textFile = textFile.Replace("$$POS", vPos);

            string vLen = textBox_MB_Len.Text;
            textFile = textFile.Replace("$$LEN", vLen);

            string vA = textBox_A.Text;
            textFile = textFile.Replace("$$A", vA);

            string vB = textBox_B.Text;
            textFile = textFile.Replace("$$B", vB);

            string vFlags = textBox_Flags.Text;
            textFile = textFile.Replace("$$FLAGS", vFlags);

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                MessageBoxUpdated("MQTT not initialized !");
                
            }

        }

        private void Button_MB_Write_HR_Click(object sender, EventArgs e)
        {
            const string template_name = @"test-write_values_hr_16.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }

            string vAlias = textBox_Alias.Text;
            textFile = textFile.Replace("$$ALIAS", vAlias);

            string vVal = textBox_HR_Val.Text;
            textFile = textFile.Replace("$$VAL", vVal);

            string vFunc = textBox_MB_HR_W_Func.Text;
            textFile = textFile.Replace("$$FUNC", vFunc);

            string vAddr = textBox_MB_Addr.Text;
            textFile = textFile.Replace("$$ADDR", vAddr);

            string vDim = textBox_MB_Dim.Text;
            textFile = textFile.Replace("$$DIM", vDim);

            string vPos = textBox_MB_Pos.Text;
            textFile = textFile.Replace("$$POS", vPos);

            string vLen = textBox_MB_Len.Text;
            textFile = textFile.Replace("$$LEN", vLen);

            string vA = textBox_A.Text;
            textFile = textFile.Replace("$$A", vA);

            string vB = textBox_B.Text;
            textFile = textFile.Replace("$$B", vB);

            string vFlags = textBox_Flags.Text;
            textFile = textFile.Replace("$$FLAGS", vFlags);

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                MessageBoxUpdated("MQTT Not Initialized");
            }

        }

        private void Button_MB_Read_COIL_Click(object sender, EventArgs e)
        {
            const string template_name = @"test-read_values_coil_1.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }

            string vAlias = textBox_Alias_Coil.Text;
            textFile = textFile.Replace("$$ALIAS", vAlias);

            //string vVal = textBox_HR_Val.Text;
            //textFile = textFile.Replace("$$VAL", vVal);

            string vFunc = textBox_MB_COIL_R_Func.Text;
            textFile = textFile.Replace("$$FUNC", vFunc);

            string vAddr = textBox_MB_Addr_Coil.Text;
            textFile = textFile.Replace("$$ADDR", vAddr);

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                MessageBoxUpdated("MQTT Not Initialized");
            }
        }

        private void Button_MB_Write_COIL_Click(object sender, EventArgs e)
        {
            const string template_name = @"test-write_values_coil_15.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }

            string vAlias = textBox_Alias_Coil.Text;
            textFile = textFile.Replace("$$ALIAS", vAlias);

            string vVal = textBox_MB_COIL_Val.Text;
            textFile = textFile.Replace("$$VAL", vVal);

            string vFunc = textBox_MB_COIL_W_Func.Text;
            textFile = textFile.Replace("$$FUNC", vFunc);

            string vAddr = textBox_MB_Addr_Coil.Text;
            textFile = textFile.Replace("$$ADDR", vAddr);

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                MessageBoxUpdated("Not Initialized");
            }
        }

        private void Button_MB_Read_DI_Click(object sender, EventArgs e)
        {
            const string template_name = @"test-read_values_di_02.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }

            string vAlias = textBox_Alias_DI.Text;
            textFile = textFile.Replace("$$ALIAS", vAlias);

            string vFunc = textBox_MB_DI_R_Func.Text;
            textFile = textFile.Replace("$$FUNC", vFunc);

            string vAddr = textBox_MB_Addr_DI.Text;
            textFile = textFile.Replace("$$ADDR", vAddr);

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                MessageBoxUpdated("Not Initialized");
            }
        }

        private void Button_MB_Read_IR_Click(object sender, EventArgs e)
        {
            
            const string template_name = @"test-read_values_ir_04.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }

            string vAlias = textBox_Alias_IR.Text;
            textFile = textFile.Replace("$$ALIAS", vAlias);

            string vFunc = textBox_MB_IR_R_Func.Text;
            textFile = textFile.Replace("$$FUNC", vFunc);

            string vAddr = textBox_MB_Addr_IR.Text;
            textFile = textFile.Replace("$$ADDR", vAddr);

            string vDim = textBox_MB_Dim_IR.Text;
            textFile = textFile.Replace("$$DIM", vDim);

            string vPos = textBox_MB_Pos_IR.Text;
            textFile = textFile.Replace("$$POS", vPos);

            string vLen = textBox_MB_Len_IR.Text;
            textFile = textFile.Replace("$$LEN", vLen);

            string vA = textBox_A_IR.Text;
            textFile = textFile.Replace("$$A", vA);

            string vB = textBox_B_IR.Text;
            textFile = textFile.Replace("$$B", vB);

            string vFlags = textBox_Flags_IR.Text;
            textFile = textFile.Replace("$$FLAGS", vFlags);

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                MessageBoxUpdated("Not Initialized");
            }



        }

        private void Button_JSON_Validate_Resp_Click(object sender, EventArgs e)
        {
            MessageBoxInfo("use CTRL-V to paste the JSON", "Info");
            System.Windows.Forms.Clipboard.SetText(txtConsole.Text);
            System.Diagnostics.Process.Start(JSON_VALIDATOR);
            //SendKeys.Send("^V");
        }

        private void Button_JSON_Validate_Pub_Click(object sender, EventArgs e)
        {
            MessageBoxInfo("use CTRL-V to paste the JSON", "Info");
            System.Windows.Forms.Clipboard.SetText(textBoxPublish.Text);
            System.Diagnostics.Process.Start(JSON_VALIDATOR);
            //SendKeys.Send("^V");
        }

        private void Button_JSON_Validate_Offline_Click(object sender, EventArgs e)
        {

            string jsonString = txtConsole.Text;
            try
            {
                //var tmpObj = JsonValue.Parse(jsonString);
            }
            catch (FormatException fex)
            {
                //Invalid json format
                MessageBoxUpdated("Res MQTT NOT valid ! " + fex.ToString());
            }
            catch (Exception ex) //some other exception
            {
                
                MessageBoxUpdated("Res MQTT is valid !");
            }


            
        }

        private void Button_res2res_reset_Click(object sender, EventArgs e)
        {
            start_stop_res2res_flag = false;
            string nstr = @"-----";
            txtConsole.Invoke(new Action(() => textBox_Res2Res.Text = nstr));
        }

        private void Button_test_set_gw_config_req_Click(object sender, EventArgs e)
        {
            string textFilePath = @"test-set_gw_config-req.json";
            PublishTestFile(textFilePath);
        }

        private void Label50_Click(object sender, EventArgs e)
        {

        }

        private void Button_start_engine_Click(object sender, EventArgs e)
        {
            string textFilePath = @"test-start_engine_req.json";
            PublishTestFile(textFilePath);
        }

        private void Button_stop_engine_Click(object sender, EventArgs e)
        {            
            string textFilePath = @"test-stop_engine_req.json";
            PublishTestFile(textFilePath);

        }

        private void Label51_Click(object sender, EventArgs e)
        {

        }

        private void TextBoxMQTT_Server_Port_TextChanged(object sender, EventArgs e)
        {

        }

        private void CheckBox_TLS_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox_TLS.Checked == true)
            {
                textBoxMQTT_Server_Port.Text = "8883";
            }
            else
            {
                textBoxMQTT_Server_Port.Text = "1883";
            }          
        }

        private void CheckBox_ShowMsg_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void Button_Save_settings_Click(object sender, EventArgs e)
        {
            string cfg_file;

            if (checkBox_Cfg_Dbg_Rel.Checked == true)
            {
                cfg_file = @"MqttClientSimulatorBinary_REL_Set.ini";
            }
            else
            {
                cfg_file = @"MqttClientSimulatorBinary_DBG_Set.ini";
            }

            var MyIni = new IniFile(cfg_file);


            MyIni.Write("Server", textBoxMQTT_Server_URL.Text);
            MyIni.Write("Port", textBoxMQTT_Server_Port.Text);

            if (checkBox_TLS.Checked == true)
            {
                MyIni.Write("TLS", "1");
            }
            else
            {
                MyIni.Write("TLS", "0");
            }


            MyIni.Write("Target", textBox_Target.Text);

        }

        private void TextBoxMQTT_Server_URL_TextChanged(object sender, EventArgs e)
        {

        }

        private void CheckBox_Cfg_Dbg_Rel_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void Button_Load_settings_Click(object sender, EventArgs e)
        {
            string cfg_file;
            string par_val;


            if (checkBox_Cfg_Dbg_Rel.Checked == true)
            {
                cfg_file = @"MqttClientSimulatorBinary_REL_Set.ini";
            }
            else
            {
                cfg_file = @"MqttClientSimulatorBinary_DBG_Set.ini";
            }

            var MyIni = new IniFile(cfg_file);

            par_val = MyIni.Read("Server");
            textBoxMQTT_Server_URL.Text = par_val;

            par_val = MyIni.Read("Port");
            textBoxMQTT_Server_Port.Text = par_val;

            par_val = MyIni.Read("TLS");

            if (par_val.Equals("1"))
            {
                checkBox_TLS.Checked = true;
            }
            else
            {
                checkBox_TLS.Checked = false;
            }
                                  

            par_val = MyIni.Read("Target");
            textBox_Target.Text = par_val;
        }

        private void Button_send_mb_adu_Click_1(object sender, EventArgs e)
        {
            const string template_name = @"test-send_mb_adu-req.json";
            string textFile = null;

            if (File.Exists(template_name))
            {
                // Read entire text file content in one string  
                //textFile = File.ReadAllText(testfilepath);
                string[] lines = File.ReadAllLines(template_name);
                textFile = String.Join(Environment.NewLine, lines);
            }
            else
            {
                textBoxPublish.Text = "ERROR " + template_name;
                return;
            }

            textBoxPublish.Text = textFile;

            try
            {
                ushort msgId = client.Publish(val_req_post, Encoding.UTF8.GetBytes(textFile), MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
                textBoxPublish.Text = textFile;
            }
            catch (System.NullReferenceException)
            {
                //Debug.WriteLine("Not Initialized");
                MessageBoxUpdated("MQTT Not Initialized");
            }
        }

    }
    }







//string textFilePath = @"C:\\hwfwdept_proj\\c780_gme_library\\Test\\Test_cases\\JSON_test_cases\\test-write_values_coil_15.json";

//string textFilePath = @"C:\\hwfwdept_proj\\c780_gme_library\\Test\\Test_cases\\JSON_test_cases\\test-write_values_hr_16-req.json";
