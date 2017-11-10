using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

namespace ACE_FILE_SERVER_VIEW
{
    public partial class MinForm : Form
    {
        public MinForm()
        {
            InitializeComponent();
            this.panel_Left.Enabled = true;
            this.panel_Right.Enabled = false;
            this.label_RemoteHint.Enabled = false;
                       
            if(this.listView_Local.Items.Count == 0)
                Btn_LocalMainDir_Click(null, null);

            richTextBox_log.AppendText("日志:\r\n");

            //OnFWndCallBack += this.FWndCallBackImpl;
            ComboBox_Connect.SelectedIndex = 0;
         }
        public void ConnectDeviceClient()
        {
            this.panel_Left.Enabled = false;
            this.panel_Right.Enabled = false;
            this.label_RemoteHint.Enabled = false;
            this.Btn_connectP2PFileTransLant.Enabled = false;
            this._isConnected = false;
        }
        public void ClosedP2pFileTranslant()
        {
            this.Btn_connectP2PFileTransLant.Enabled = false;
        }
        private void SetMainTainLogId(string logId)
        {
            _logId = logId;
        }


        #region 更新通知事件

        #region 服务器消息回执通知事件
        //更新设备列表        
        #endregion

        #region 文件发送和接收事件处理
        public void OnUploadCompleteImpl(string filePath)
        {
                string fileName = Path.GetFileName(filePath);
                this.WriteLog(false, string.Format("文件\"{0}\"发送完成", filePath));
                //有可能会有临时文件产生，这里刷新下列表(同名时)
                Btn_RemoteRush_Click(this.Btn_RemoteRush, new EventArgs());
        }

        //接收到文件列表
        public void OnGetFileListImpl(string fileList)
        {
            try
            {
                    string dir = "<DFList>";
                    string dirNode = "</DList>";
                    string fileNode = "</FList>";
                    string xml = fileList;
                    int index = xml.IndexOf("<DList>");
                    int xmllen = xml.Length;
                    int length = xml.IndexOf("</DList>") - dir.Length + dirNode.Length;
                    List<FileDirInfomation.Ditm> DirList = new List<FileDirInfomation.Ditm>();
                    List<FileDirInfomation.Fitm> FileList = new List<FileDirInfomation.Fitm>();
                    
                    if (index > 0 && index < xmllen && length < xmllen)
                    {
                        string xmlRe1 = xml.Substring(index,
                                                      length);
                        if (xmlRe1.IndexOf("&") != -1)
                            xmlRe1 = xmlRe1.Replace("&", "&#38;");//转义字符
                        DirList = XmlHelper.Deserialize<FileDirInfomation.Ditm>(xmlRe1, "DList");
                    }
                    else
                    {
                        DirList.Clear();
                    }

                    index = xml.IndexOf("<FList>");
                    length = xml.IndexOf("</FList>") - index + fileNode.Length;
                    if (index > 0 && index < xmllen && length < xmllen)
                    {
                        string xmlRe2 = xml.Substring(index,
                                                      length);
                        if (xmlRe2.IndexOf("&") != -1)
                            xmlRe2 = xmlRe2.Replace("&", "&#38;");//转义字符
                        FileList = XmlHelper.Deserialize<FileDirInfomation.Fitm>(xmlRe2, "FList");
                    }
                    else
                    {
                        FileList.Clear();
                    }

                    RemoteFileDir.DirLists = DirList;
                    RemoteFileDir.FileLists = FileList;

                    this.panel_Right.Enabled = true;
                    this.label_RemoteHint.Enabled = true;
                    this.label_RemoteHint.Text = "获取目录列表完成";
                    UpdateDirFilesToListView(false);

                    if (this.TextBox_RemotoPath.Items.IndexOf(this.TextBox_RemotoPath.Text) == -1)
                        this.TextBox_RemotoPath.Items.Add(this.TextBox_RemotoPath.Text);
            }
            catch (System.Exception ex)
            {
                this.WriteLog(false, string.Format("更新远程目录出错:{0}", ex.Message));
            }              
        }
        #endregion

        #endregion
        
        #region 连接事件处理    
        private void Btn_Connected_Click(object sender, EventArgs e)
        {
            if(Btn_Connected.Text == "连接服务器")
            {
                try
                {
                    timer_ConnectedServer_Tick(null, null);
                    if (_isConnected)                        
                    {
                        MessageBox.Show("连接服务器成功!");
                        timer_ConnectedServer.Enabled = true;
                    }
                    else
                        MessageBox.Show("连接服务器失败!");                    
                }
                catch (System.Exception ex)
                {
                    MessageBox.Show(string.Format("登录出现异常,错误信息如下:\r\n{0}\r\n{1}",ex.Message,ex.StackTrace), "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }else
            {
                DisConnect();
                timer_ConnectedServer.Enabled = false;
            }
        }
        private void timer_ConnectedServer_Tick(object sender, EventArgs e)
        {
            int Port = 0;
            try
            {
                Port = Convert.ToInt32(this.Text_Port.Text);
            }
            catch (System.Exception ex)
            {
                Port = 21;
            }

            CurFileServerPtr.InitConnected(this.Text_IP.Text, Port, this.Text_UserName.Text, this.Text_Password.Text);
            int iConnectCount = CurFileServerPtr.connectServer();
            if(iConnectCount == -1)
            {
                if(_isConnected)
                    DisConnect();
                timer_ConnectedServer.Enabled = false;
            }
            else
            {
                if(!_isConnected)
                    Connected();
            }
            Label_ServerCurCount.Text = string.Format("服务器当前连接数:{0}", iConnectCount);
        }        
        /// <summary>
        /// 断开连接
        /// </summary>
        private delegate void DisConnectDelegate();
        private void DisConnect()
        {
            if (InvokeRequired)
            {
                DisConnectDelegate disConnectDelegate = DisConnect;
                Invoke(disConnectDelegate);
            }
            else
            {
                Btn_Connected.Text = "连接服务器";
                this.Btn_connectP2PFileTransLant.Enabled = true;
                this.Btn_connectP2PFileTransLant.Text = "连接服务器";
                this.Label_ConnectTime.Text = "启动时间:";
                this.listView_Remote.Items.Clear();

                panel_Right.Enabled = false;
                this.label_RemoteHint.Enabled = false;
                this._isConnected = false;
                this.Text_IP.Enabled = true;
                this.Text_Port.Enabled = true;

                Text_UserName.Enabled = true;
                Text_Password.Enabled = true;
                this.ComboBox_Connect.Enabled = true;
                this.WriteLog(false, "连接已断开.");
            }
        }
        /// <summary>
        /// 连接成功
        /// </summary>
        private delegate void ConnectedDelegate();
        private void Connected()
        {
                if (InvokeRequired)
                {
                    ConnectedDelegate dtConnectedDelegate = Connected;
                    Invoke(dtConnectedDelegate);
                }
                else
                {
                    Btn_Connected.Text = "断开服务器";
                    panel_Right.Enabled = true;
                    this.label_RemoteHint.Enabled = true;
                    this._isConnected = true;
                    this.Text_IP.Enabled = false;
                    this.Text_Port.Enabled = false;
                    Text_UserName.Enabled = false;
                    Text_Password.Enabled = false;
                    this.ComboBox_Connect.Enabled = false;
                    this.WriteLog(false, "已建立连接.");

                    this.Btn_connectP2PFileTransLant.Enabled = true;
                    this.Btn_connectP2PFileTransLant.Text = "断开服务器";
                    this.Label_ConnectTime.Text = string.Format("启动时间:{0}", DateTime.Now.ToLocalTime());                    

                    //获取远程驱动器列表
                    if (this.listView_Remote.Items.Count == 0)
                    Btn_RemoteMainDir_Click(null, new EventArgs());
                }
        }       
        #endregion


        #region 文件传送命令
        /// <summary>
        /// 接收到服务返回来的消息。
        /// </summary>
        public void FWndCallBackImpl(int callBackType, ref ACE_FILE_SERVER_DLL.FWndCallBackObject sender)
        {
            if (InvokeRequired)
            {
                ACE_FILE_SERVER_DLL.FWndCallBackHandle callback = FWndCallBackImpl;
                //这里将sender.isSucessed值修改后返回到调用处，当跨线程使用Invoke后就返回不了值了，这里先就先对需要返回值的地方进行处理;
                switch (callBackType)
                {
                    case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_SEND_ISOVERWRITE:
                        {
                            string str = Marshal.PtrToStringAnsi(sender.CMDMsg);
                            sender.isSucessed = (MessageBox.Show(str, "提示", MessageBoxButtons.OKCancel) == DialogResult.OK);
                            return;
                        }
                    case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_ISOVERWRITE:
                        {
                            string str = Marshal.PtrToStringAnsi(sender.CMDMsg);
                            sender.isSucessed = (MessageBox.Show(str, "提示", MessageBoxButtons.OKCancel) == DialogResult.OK);
                            return;
                        }
                }
                Invoke(callback, new object[] { callBackType, sender });
                return;
            }

            this.panel_Right.Enabled = true;
            this.label_RemoteHint.Enabled = true;
            this.listView_Remote.Enabled = true;
            this.ToolBar_Remote.Enabled = true;
            switch (callBackType)
            {
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_PROGRESS:
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_SEND_PROGRESS:
                    {
                        int index = this.FileUpDownListView.Items.IndexOfKey(sender.HANDLE.ToString());
                        if (index != -1)
                            this.FileUpDownListView.Items[index].SubItems[4].Text = string.Format("%{0}", sender.iProgress);
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_SEND_SUCESSED:
                    {
                        int index = this.FileUpDownListView.Items.IndexOfKey(sender.HANDLE.ToString());
                        if (index != -1)
                        {                            
                            this.FileUpDownListView.Items[index].SubItems[4].Text = "发送完成";
                            this.WriteLog(false, string.Format("文件\"{0}\"发送完成.", this.FileUpDownListView.Items[index].SubItems[1].Text));
                            this.Btn_RemoteRush_Click(null, null);
                        }
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_SEND_FAILED:
                    {
                        int index = this.FileUpDownListView.Items.IndexOfKey(sender.HANDLE.ToString());
                        if (index != -1)
                        {                            
                            this.FileUpDownListView.Items[index].SubItems[4].Text = "发送失败";
                            this.WriteLog(false, string.Format("文件\"{0}\"发送失败.", this.FileUpDownListView.Items[index].SubItems[1].Text));

                            this.Btn_RemoteRush_Click(null, null);
                        }
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_SUCESSED:
                    {
                        int index = this.FileUpDownListView.Items.IndexOfKey(sender.HANDLE.ToString());
                        if (index != -1)
                        {                            
                            this.FileUpDownListView.Items[index].SubItems[4].Text = "下载完成";
                            this.WriteLog(false, string.Format("文件\"{0}\"接收完成.", this.FileUpDownListView.Items[index].SubItems[1].Text));
                            this.tbn_LocalRush_Click(null, null);
                        }
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_FAILED:
                    {
                        int index = this.FileUpDownListView.Items.IndexOfKey(sender.HANDLE.ToString());
                        if (index != -1)
                        {                            
                            this.FileUpDownListView.Items[index].SubItems[4].Text = "下载失败";
                            this.WriteLog(false, string.Format("文件\"{0}\"下载失败.", this.FileUpDownListView.Items[index].SubItems[1].Text));
                            this.tbn_LocalRush_Click(null, null);
                        }
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_FILELIST:
                    {
                        string str = Marshal.PtrToStringAnsi(sender.CMDMsg);
                        this.OnGetFileListImpl(str);
                        this.WriteLog(false, "目录已更新.");
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_DIRDEL:
                    if (sender.isSucessed)
                    {
                        this.WriteLog(false, "目录删除成功.");
                    }
                    else
                    {
                        this.WriteLog(false, "目录删除失败.");
                    }
                    this.Btn_RemoteRush_Click(null, null);
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_FILEDEL:
                    if (sender.isSucessed)
                    {
                        this.WriteLog(false, "文件删除成功.");
                        this.Btn_RemoteRush_Click(null, null);
                    }
                    else
                    {
                        this.WriteLog(false, "文件删除失败.");
                    }
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_DIRCREATE:
                    if (sender.isSucessed)
                    {
                        this.WriteLog(false, "目录创建成功.");
                    }
                    else
                    {
                        this.WriteLog(false, "目录创建失败.");
                    }
                    this.Btn_RemoteRush_Click(null, null);
                    break;
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_SEND_ISOVERWRITE:
                    {
                        string str = Marshal.PtrToStringAnsi(sender.CMDMsg);
                        sender.isSucessed = (MessageBox.Show(str, "提示", MessageBoxButtons.OKCancel) == DialogResult.OK);
                        break;
                    }
                case ACE_FILE_SERVER_DLL.CALLBACK_TYPE_RECV_ISOVERWRITE:
                    {
                        string str = Marshal.PtrToStringAnsi(sender.CMDMsg);
                        sender.isSucessed = (MessageBox.Show(str, "提示", MessageBoxButtons.OKCancel) == DialogResult.OK);
                        break;
                    }

            }//switch
        }

        FileServerBase CurFileServerPtr;
        ACE_FILE_SERVER_DLL.FWndCallBackHandle OnFWndCallBack;
        public void P2PFile_GetDeviceListCmd()
        {
            CurFileServerPtr.recvFileListFramServer("\\");
        }
        public void P2PFile_GetDirListByPathCmd(string path)
        {
            CurFileServerPtr.recvFileListFramServer(path);
        }
        public void P2PFile_CreateDirCmd(string DirPath)
        {
            CurFileServerPtr.createDirFramServer(DirPath);
        }
        public void P2PFile_DelDirCmd(string DirPath)
        {
            CurFileServerPtr.delDirFramServer(DirPath);
        }
        public void P2PFile_DelFileCmd(string FilePath)
        {
            CurFileServerPtr.delFileFramServer(FilePath);
        }
        public void P2PFile_RenameDirCmd(string SrcDirName, string DesDirName)
        {
            ;//
        }
        public void P2PFile_RenameFileCmd(string SrcFileName, string DesFileName)
        {
            ;//
        }
        public void P2pFile_SendFileCmd(string LocalfileNamePath, string RemoteDirPath)
        {
            foreach (ListViewItem item in this.listView_Local.SelectedItems)
            {
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (FItem != null)
                {
                    int iHandle = CurFileServerPtr.sendFileToServer(LocalfileNamePath, RemoteDirPath);

                    if(iHandle !=  -1)
                    {
                        Icon ico = P2pFileTransLantHelper.GetIcon(LocalfileNamePath, false);
                        if (ico != null)
                            FileUpDownListView.SmallImageList.Images.Add(ico);

                        ListViewItem UPitem = FileUpDownListView.Items.Add(iHandle.ToString(),
                                                                      "上传",
                                                                      FileUpDownListView.SmallImageList.Images.Count - 1);
                        UPitem.SubItems.Add(Path.GetFileName(LocalfileNamePath));
                        UPitem.SubItems.Add(FItem.size);
                        UPitem.SubItems.Add(FItem.GetTypeStr());
                        UPitem.SubItems.Add("%0");
                        UPitem.SubItems.Add(RemoteDirPath);
                        UPitem.SubItems.Add(iHandle.ToString());
                    }
                    else
                    {
                        MessageBox.Show("接收失败!"); ;
                    }                  
                }
            }
        }
        public void P2pFile_RecvFileCmd(string RemotefileNamePath, string LocalDirPath)
        {
            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (FItem != null)
                {
                    int iHandle = CurFileServerPtr.recvFileFramServer(RemotefileNamePath, LocalDirPath);

                    if (iHandle != -1)
                    {
                        Icon ico = P2pFileTransLantHelper.GetIcon(Path.GetFileName(RemotefileNamePath), false);
                        if (ico != null)
                            FileUpDownListView.SmallImageList.Images.Add(ico);

                        ListViewItem UPitem = FileUpDownListView.Items.Add(iHandle.ToString(),
                                                                      "下载",
                                                                      FileUpDownListView.SmallImageList.Images.Count - 1);
                        UPitem.SubItems.Add(Path.GetFileName(RemotefileNamePath));
                        UPitem.SubItems.Add(FItem.size);
                        UPitem.SubItems.Add(FItem.GetTypeStr());
                        UPitem.SubItems.Add("%0");
                        UPitem.SubItems.Add(LocalDirPath);
                        UPitem.SubItems.Add(iHandle.ToString());
                    }
                    else
                    {
                        MessageBox.Show("接收失败!"); ;
                    }
                }
            }
        }
        #endregion

        //属性
        private FileDirInfomation LocalFileDir = new FileDirInfomation();
        private FileDirInfomation RemoteFileDir = new FileDirInfomation();
        private bool _isConnected = false;
        private string _logId = "";

        #region 基础方法
        //获取全路径名称
        public string GetFullFilePathByFileItem(FileDirInfomation.Fitm file, bool isLocal)
        {
            if (isLocal)
            {
                if (3 == LocalFileDir.CurDir.Length)
                    return LocalFileDir.CurDir + file.fp;
                else
                    return LocalFileDir.CurDir + "\\" + file.fp;
            }
            else
            {
                if (1 == RemoteFileDir.CurDir.Length)
                    return RemoteFileDir.CurDir + file.fp;
                else
                    return RemoteFileDir.CurDir + "\\" + file.fp;
            }             
        }
        //获取目录全路径名称
        public string GetFullDirPathByDirItem(FileDirInfomation.Ditm dir, bool isLocal)
        {
            if (isLocal)
            {
                if (3 == LocalFileDir.CurDir.Length)
                    return LocalFileDir.CurDir + dir.dp;
                else
                    return LocalFileDir.CurDir + "\\" + dir.dp;
            }
            else
            {
                if (1 == RemoteFileDir.CurDir.Length)//根目录
                    return RemoteFileDir.CurDir + dir.dp;
                else
                    return RemoteFileDir.CurDir + "\\" + dir.dp;
            }
        }
        //获取全路径
        public string GetFullPath(bool isLocal)
        {
            if (isLocal)
            {
                if (3 == LocalFileDir.CurDir.Length)
                    return LocalFileDir.CurDir;             //根目录
                else if (1 == LocalFileDir.CurDir.Length)
                    return LocalFileDir.CurDir;             //驱动器目录
                else if (LocalFileDir.CurDir.Length > 0 && LocalFileDir.CurDir[LocalFileDir.CurDir.Length - 1] == '\\')
                    return LocalFileDir.CurDir;      //子目录
                else
                    return LocalFileDir.CurDir + "\\";      //子目录
            }
            else
            {
                if (3 == RemoteFileDir.CurDir.Length)       //根目录
                    return RemoteFileDir.CurDir;
                else if (1 == RemoteFileDir.CurDir.Length)  //驱动器目录
                    return RemoteFileDir.CurDir;
                else if (RemoteFileDir.CurDir.Length > 0 && RemoteFileDir.CurDir[RemoteFileDir.CurDir.Length - 1] == '\\')
                    return RemoteFileDir.CurDir;            //子目录
                else
                    return RemoteFileDir.CurDir + "\\";     //子目录
            }
        }
        public string GetCurDirParent(bool isLocal)
        {
            string ret = "";
            if (isLocal)
                ret = Path.GetDirectoryName(LocalFileDir.CurDir);
            else
            {
                int index = RemoteFileDir.CurDir.LastIndexOf("\\");
                int icount = RemoteFileDir.CurDir.Length;
                ret = RemoteFileDir.CurDir.Substring(0,index);
            }
            return ret == null ? "":ret;
        }
        //当前目录是否在驱动器目录
        public bool CurDirIsDevicesRoot(bool isLocal)
        {
                return 1 == GetFullPath(isLocal).Length;
        }
        //当前目录是否在盘符根目录
        public bool CurDirIsLogDiskRoot(bool isLocal)
        {
            if (isLocal)
                return 3 == GetFullPath(isLocal).Length;
            else
                return 1 == GetFullPath(isLocal).Length; 
        }


        //更新目录(本地/远程),并更新界面
        public void UpdateDevicesToListView(bool isLocal)
        {
            FileDirInfomation fileDir;
            ListView ls;

            if (isLocal)
            {
                fileDir = this.LocalFileDir;
                ls = this.listView_Local;
                this.TextBox_localPath.Text = fileDir.CurDir;
            }
            else
            {
                fileDir = this.RemoteFileDir;
                ls = this.listView_Remote;
                this.TextBox_RemotoPath.Text = fileDir.CurDir;
            }


            ls.BeginUpdate();
            try
            {
                ls.SmallImageList.Images.Clear();
                ls.Items.Clear();
                foreach (FileDirInfomation.Device device in fileDir.Devices)
                {
                    Icon ico = P2pFileTransLantHelper.GetIcon(device.name, false);
                    if (ico != null)
                        ls.SmallImageList.Images.Add(ico);

                    ListViewItem item = ls.Items.Add("",
                                                    device.name, ls.SmallImageList.Images.Count - 1);
                    item.SubItems.Add("");
                    item.SubItems.Add(device.type);
                    item.SubItems.Add("");
                    item.Tag = device;
                }
            }
            finally
            {
                ls.EndUpdate();
            }

            listView_Local_SelectedIndexChanged(this.listView_Local, new EventArgs());
            listView_Remote_SelectedIndexChanged(this.listView_Remote, new EventArgs());
        }
        public void UpdateDirFilesToListView(bool isLocal)
        {
            FileDirInfomation fileDir;
            ListView ls;

            if (isLocal)
            {
                fileDir = this.LocalFileDir;
                ls = this.listView_Local;
                this.TextBox_localPath.Text = fileDir.CurDir;
            }
            else
            {
                fileDir = this.RemoteFileDir;
                ls = this.listView_Remote;
                this.TextBox_RemotoPath.Text = fileDir.CurDir;
            }


            ls.BeginUpdate();
            try
            {
                ls.SmallImageList.Images.Clear();
                ls.Items.Clear();
                ls.SmallImageList.Images.Add(Btn_LocalMainDir.Image);
                foreach (FileDirInfomation.Ditm dir in fileDir.DirLists)
                {
                    ListViewItem item = ls.Items.Add("",
                                                     dir.dp,
                                                     ls.SmallImageList.Images.Count - 1);
                    item.SubItems.Add("");
                    item.SubItems.Add("文件夹");
                    item.SubItems.Add(dir.ct);
                    item.Tag = dir;
                }

                foreach (FileDirInfomation.Fitm file in fileDir.FileLists)
                {
                    Icon ico = P2pFileTransLantHelper.GetIcon(file.fp, false);
                    if (ico != null)
                        ls.SmallImageList.Images.Add(ico);

                    ListViewItem item = ls.Items.Add("",
                                                    file.fp, ls.SmallImageList.Images.Count - 1);
                    item.SubItems.Add(file.size);
                    item.SubItems.Add(file.GetTypeStr());
                    item.SubItems.Add(file.ct);
                    item.Tag = file;
                }
            }
            finally
            {
                ls.EndUpdate();
            }

            listView_Local_SelectedIndexChanged(this.listView_Local, new EventArgs());
            listView_Remote_SelectedIndexChanged(this.listView_Remote, new EventArgs());
        }

        //当前操作的目录名或文件名界面元素
        ListViewItem CurListViewItem;

        #endregion


        #region 界面事件
        //打开某一个目录(本地)
        private void listView_Local_DoubleClick(object sender, EventArgs e)
        {
            string lastDir = this.LocalFileDir.CurDir;
            try
            {
                foreach (ListViewItem item in this.listView_Local.SelectedItems)
                {
                    FileDirInfomation.Device device = item.Tag as FileDirInfomation.Device;
                    FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                    if (device != null || dItem != null)
                    {
                        if (device != null)
                            this.LocalFileDir.CurDir = device.name;
                        else
                            this.LocalFileDir.CurDir = GetFullDirPathByDirItem(dItem, true);

                        this.label_LocalHint.Text = "正在获取目录内容";
                        UpdateLocalDirListByDirPath();
                        this.label_LocalHint.Text = "目录内容获取完成";
                        this.CurListViewItem = null;
                        break;
                    }
                }
            }
            catch (Exception ex)
            {
                this.LocalFileDir.CurDir = lastDir;
                this.WriteLog(true,string.Format("获取目录内容失败，错误信息:{0}",ex.Message));
                this.label_LocalHint.Text = "目录内容获取完成";
            }
        }
        //更新本地目录列表
        private void UpdateLocalDirListByDirPath()
        {
            string dir = "<DFList>";
            string dirNode = "</DList>";
            string fileNode = "</FList>";
            string xml = FileDirHelper.GetDirListByPathXml(this.LocalFileDir.CurDir);
            int index = xml.IndexOf("<DList>");
            int xmllen = xml.Length;
            int length = xml.IndexOf("</DList>") - dir.Length + dirNode.Length;
            if (index > 0 && index < xmllen && length < xmllen)
            {
                string xmlRe1 = xml.Substring(index,
                                              length);
                if (xmlRe1.IndexOf("&") != -1)
                    xmlRe1 = xmlRe1.Replace("&", "&#38;");//转义字符
                List<FileDirInfomation.Ditm> DirList = XmlHelper.Deserialize<FileDirInfomation.Ditm>(xmlRe1, "DList");
                this.LocalFileDir.DirLists = DirList;
            }
            else
            {
                this.LocalFileDir.DirLists.Clear();
            }

            index = xml.IndexOf("<FList>");
            length = xml.IndexOf("</FList>") - index + fileNode.Length;
            if (index > 0 && index < xmllen && length < xmllen)
            {
                string xmlRe2 = xml.Substring(index,
                                              length);
                if (xmlRe2.IndexOf("&") != -1)
                    xmlRe2 = xmlRe2.Replace("&", "&#38;");//转义字符
                List<FileDirInfomation.Fitm> FileList = XmlHelper.Deserialize<FileDirInfomation.Fitm>(xmlRe2, "FList");
                this.LocalFileDir.FileLists = FileList;
            }
            else
            {
                this.LocalFileDir.FileLists.Clear();
            }
            this.UpdateDirFilesToListView(true);
            this.CurListViewItem = null;

            if (this.TextBox_localPath.Items.IndexOf(this.TextBox_localPath.Text) == -1)
                this.TextBox_localPath.Items.Add(this.TextBox_localPath.Text);
        }
        //打开另一个目录(远程)
        private void listView_Remote_DoubleClick(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                FileDirInfomation.Device device = item.Tag as FileDirInfomation.Device;
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                if (device != null || dItem != null)
                {
                    if (device != null)
                        this.RemoteFileDir.CurDir = device.name;
                    else
                        this.RemoteFileDir.CurDir = GetFullDirPathByDirItem(dItem,false);

                    this.panel_Right.Enabled = false;
                    this.label_RemoteHint.Enabled = false;
                    this.label_RemoteHint.Text = "正在获取目录内容.请稍等...";
                    P2PFile_GetDirListByPathCmd(this.RemoteFileDir.CurDir);
                }
            }
        }
        //主目录(本地)
        private void Btn_LocalMainDir_Click(object sender, EventArgs e)
        {
            try
            {
                string XML = FileDirHelper.GetDeviceListXml();
                List<FileDirInfomation.Device> DList = XmlHelper.Deserialize<FileDirInfomation.Device>(XML, "DeviceList");
                this.LocalFileDir.CurDir = "\\";
                this.LocalFileDir.Devices = DList;
                this.UpdateDevicesToListView(true);

                this.CurListViewItem = null;
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("获取驱动列表信息出错，请重试!\r\n错误信息:{0}",ex.Message), "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

        }
        //主目录(远程)
        private void Btn_RemoteMainDir_Click(object sender, EventArgs e)
        {
            try
            {
                this.panel_Right.Enabled = false;
                this.label_RemoteHint.Enabled = false;
                this.label_RemoteHint.Text = "正在获取内容.请稍等...";
                this.RemoteFileDir.CurDir = "\\";
                P2PFile_GetDeviceListCmd();

                this.CurListViewItem = null;
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("获取服务器列表信息出错，请重试!\r\n错误信息:{0}", ex.Message), "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
        //返回(本地)
        private void btn_LocalReturn_Click(object sender, EventArgs e)
        {
            try
            {
                if (this.CurDirIsDevicesRoot(true) || this.CurDirIsLogDiskRoot(true))
                {
                    this.Btn_LocalMainDir_Click(null, new EventArgs());
                }
                else
                {
                    this.LocalFileDir.CurDir = this.GetCurDirParent(true);
                    if (!string.IsNullOrEmpty(this.LocalFileDir.CurDir))
                        this.UpdateLocalDirListByDirPath();
                }                    
            }
            catch (Exception ex)
            {
                this.WriteLog(true, string.Format("获取目录内容出错，请重试! 错误信息:{0}", ex.Message));                                               
            }
        }
        //远程防问
        private void btn_RemotoReturn_Click(object sender, EventArgs e)
        {
            try
            {
                this.panel_Right.Enabled = false;
                this.label_RemoteHint.Enabled = false;
                if (this.CurDirIsDevicesRoot(false) || this.CurDirIsLogDiskRoot(false))
                {
                    this.Btn_RemoteMainDir_Click(null, new EventArgs());
                }
                else
                {
                    this.RemoteFileDir.CurDir = this.GetCurDirParent(false);
                    this.label_RemoteHint.Text = "正在获取目录内容.请稍等...";
                    P2PFile_GetDirListByPathCmd(this.RemoteFileDir.CurDir);
                }                    
            }
            catch (Exception ex)
            {
                this.WriteLog(false, string.Format("获取目录内容出错，请重试! 错误信息:{0}", ex.Message));
            }
        }        
        //本地重命名
        private void btn_LocalRename_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listView_Local.SelectedItems)
            {
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem != null || FItem != null)
                {
                    item.BeginEdit();
                    break;
                }
            }
        }
        //本地重命名
        private void listView_Local_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.F2)
            {
                ListView ls = sender as ListView;
                foreach (ListViewItem item in ls.SelectedItems)
                {
                    FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                    FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                    if (dItem != null || FItem != null)
                    {
                        item.BeginEdit();
                        break;
                    }
                }
            }
        }
        //本地重命名操作
        private void listView_Local_BeforeLabelEdit(object sender, LabelEditEventArgs e)
        {
            ListView listview = sender as ListView;
            foreach (ListViewItem item in listview.SelectedItems)
            {
                if (item.Tag == null)//新建文件夹
                    break;

                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem == null && FItem == null)
                {
                    e.CancelEdit = true;
                    break;
                }
            }
        }
        //本地重命名操作
        private void listView_Local_AfterLabelEdit(object sender, LabelEditEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Label))
            {
                foreach (ListViewItem item in this.listView_Local.SelectedItems)
                {                    
                    if (item.Tag == null)//建立默认文件名:"新建文件夹"
                    {   //item.Remove();                        
                        string dirName = this.GetFullPath(true) + "新建文件夹";                        
                        if (FileDirHelper.CreateOrDelDir(0, dirName))
                        {
                            FileDirInfomation.Ditm dItem = new FileDirInfomation.Ditm(item.Text, DateTime.Now.ToString("yy-mm-dd hh:mm"));
                            item.Tag = dItem;
                            this.LocalFileDir.DirLists.Add(dItem);
                            dItem.dp = "新建文件夹";
                            WriteLog(true, string.Format("创建目录\"{0}\"成功", dirName));
                        }
                        else
                        {
                            e.CancelEdit = true;
                            item.Remove();
                            WriteLog(true, string.Format("创建目录\"{0}\"失败,请检查是否有同名情况", dirName));
                        }
                    }
                }
                return;
            }

            foreach (ListViewItem item in this.listView_Local.SelectedItems)
            {       
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem != null)
                {
                    string srcName = this.GetFullDirPathByDirItem(dItem, true);
                    string decName = this.GetFullPath(true) + e.Label;

                    if (FileDirHelper.RenameDirOrFile(1, srcName, decName))
                    {
                        WriteLog(true, string.Format("已修改目录\"{0}\"为\"{1}\"", srcName, decName));
                        dItem.dp = e.Label;
                    }
                    else
                    {
                        WriteLog(true, string.Format("修改目录\"{0}\"失败,请检查是否重名", srcName, decName));
                        e.CancelEdit = true;
                    }
                    break;
                }
                else if (FItem != null)
                {
                    string srcName = this.GetFullFilePathByFileItem(FItem, true);
                    string decName = this.GetFullPath(true) + e.Label;

                    if (FileDirHelper.RenameDirOrFile(0, srcName, decName))
                    {
                        WriteLog(true, string.Format("已修改文件\"{0}\"为\"{1}\"", srcName, decName));
                        FItem.fp = e.Label;
                    }
                    else
                    {
                        WriteLog(true, string.Format("修改文件名称\"{0}\"失败,请重试,注意系统文件和重名会出现更改失败的情况", srcName, decName));
                        e.CancelEdit = true;
                    }
                    break;
                }
                else if (item.Tag == null)//新建文件夹
                {
                    string dirName = this.GetFullPath(true) + e.Label;
                    if (FileDirHelper.CreateOrDelDir(0, dirName))
                    {
                        dItem = new FileDirInfomation.Ditm(item.Text, DateTime.Now.ToString("yy-mm-dd hh:mm"));
                        item.Tag = dItem;
                        this.LocalFileDir.DirLists.Add(dItem);
                        dItem.dp = e.Label;
                        WriteLog(true, string.Format("创建目录\"{0}\"成功", dirName));
                    }
                    else
                    {
                        e.CancelEdit = true;
                        item.Remove();
                        WriteLog(true, string.Format("创建目录\"{0}\"失败,请检查是否有同名情况", dirName));
                    }
                    break;
                }
            }
        }
        //日志相关
        private void WriteLog(bool isLocal,string log)
        {
            string str = string.Format("[{0}][{1}]{2}\r\n", DateTime.Now.ToLocalTime(),
                                                      isLocal ? "本地操作" : "远程操作",
                                                      log);
            richTextBox_log.AppendText(str);
            try
            {
                richTextBox_log.Focus();
                richTextBox_log.Select(richTextBox_log.TextLength, 0);
                richTextBox_log.ScrollToCaret();
            }
            catch(Exception ex)
            {

            }
        }
        //删除操作(本地)
        private void btn_LocalDel_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listView_Local.SelectedItems)
            {
                if (MessageBox.Show("删除后将不能恢复，请确定是否要进行删除操作.", "提示", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.Cancel)
                    break;
                
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem != null)
                {
                    string srcName = this.LocalFileDir.CurDir + "\\" + dItem.dp;
                    if (FileDirHelper.CreateOrDelDir(1, srcName))
                    {
                        WriteLog(true, string.Format("已删除目录\"{0}\"", srcName));
                        this.LocalFileDir.DirLists.Remove(dItem);
                        item.Remove();
                    }
                    else
                    {
                        WriteLog(true, string.Format("删除目录\"{0}\"失败,请重试", srcName));
                    }
                }
                else if (FItem != null)
                {
                    string srcName = this.LocalFileDir.CurDir + "\\" + FItem.fp;
                    if (FileDirHelper.DelFile(srcName))
                    {
                        WriteLog(true, string.Format("已删除文件\"{0}\"", srcName));
                        this.LocalFileDir.FileLists.Remove(FItem);
                        item.Remove();                        
                    }
                    else
                    {
                        WriteLog(true, string.Format("删除文件\"{0}\"失败,请检查该文件是否已打开或正在运行", srcName));
                    }
                }
            }
        }
        //刷新操作(本地)
        private void tbn_LocalRush_Click(object sender, EventArgs e)
        {
            try
            {
                if (3 <= this.LocalFileDir.CurDir.Length)
                {
                    if (this.LocalFileDir.CurDir != null)
                        this.UpdateLocalDirListByDirPath();
                }
                else
                    this.Btn_LocalMainDir_Click(null, new EventArgs());
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("获取目录信息出错，请重试!\r\n错误信息:{0}", ex.Message), "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
        //建立文件夹操作(本地)
        private void btn_LocalCreateDir_Click(object sender, EventArgs e)
        {
            if (richTextBox_log.CanFocus)
                richTextBox_log.Focus();

            ListViewItem item = listView_Local.Items.Add("",
                                                        "新建文件夹",0);
            item.SubItems.Add("");
            item.SubItems.Add("文件夹");
            item.SubItems.Add(DateTime.Now.ToString("yyy-MM-dd hh:mm"));

            item.Tag = null;
            item.Selected = true;
            item.BeginEdit();                       
        }
        //UI可用处理(本地)
        private void listView_Local_SelectedIndexChanged(object sender, EventArgs e)
        {
            this.btn_LocalDel.Enabled = false;
            this.btn_LocalRename.Enabled = false;
            this.btn_LocalReturn.Enabled = false;
            this.Btn_LocalSend.Enabled = false;
            this.btn_LocalCreateDir.Enabled = false;
            this.Btn_LocalSend.Enabled = false;

            foreach (ListViewItem item in this.listView_Local.SelectedItems)
            {
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                FileDirInfomation.Device Ditem = item.Tag as FileDirInfomation.Device;
                if (dItem != null || FItem != null)
                {
                    this.btn_LocalDel.Enabled = true;
                    this.btn_LocalRename.Enabled = true;
                    if (this.listView_Remote.Enabled && FItem != null)
                        this.Btn_LocalSend.Enabled = true;
                }
            }

            if (!this.CurDirIsDevicesRoot(true))
            {
                this.btn_LocalCreateDir.Enabled = true;
                this.btn_LocalReturn.Enabled = true;
            }


            /*if(Btn_LocalSend.Enabled)
                Btn_LocalSend.BackgroundImage = Properties.Resources.btn_bg02;
            else
                Btn_LocalSend.BackgroundImage = null;//Properties.Resources.btn_bg01_hover;*/
        }                    
        //远程lsUI可用处理(远)
        private void listView_Remote_SelectedIndexChanged(object sender, EventArgs e)
        {
            this.btn_RemotoReturn.Enabled = false;
            this.Btn_RemoteCreateDir.Enabled = true;
            this.Btn_RemoteSend.Enabled = true;
            this.Btn_RemoteDel.Enabled = false;
            this.Btn_RemoteRename.Enabled = false;
            this.Btn_RemoteSend.Enabled = false;

            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                FileDirInfomation.Device Ditem = item.Tag as FileDirInfomation.Device;
                if (dItem != null || FItem != null)
                {
                    this.Btn_RemoteDel.Enabled = true;
                    this.Btn_RemoteRename.Enabled = true;
                    if (FItem != null && !this.CurDirIsDevicesRoot(true))
                    {
                        this.Btn_RemoteSend.Enabled = true;
                    }
                }
            }

            if (!this.CurDirIsDevicesRoot(false))
            {
                this.btn_RemotoReturn.Enabled = true;
            }

            /*if (Btn_RemoteSend.Enabled)
                Btn_RemoteSend.BackgroundImage = Properties.Resources.btn_bg02;
            else
                Btn_RemoteSend.BackgroundImage = null;// Properties.Resources.btn_bg01_hover;*/
        }        
        //重命名操作(远)
        private void Btn_RemoteRename_Click(object sender, EventArgs e)
        {
            if (richTextBox_log.CanFocus)
                richTextBox_log.Focus();            
            
            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem != null || FItem != null)
                {
                    item.BeginEdit();
                    break;
                }
            }
        }
        //重命名/新建操作
        private void listView_Remote_AfterLabelEdit(object sender, LabelEditEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Label))
            {
                foreach (ListViewItem item in this.listView_Remote.SelectedItems)
                {
                    if (item.Tag == null)//建立默认文件名:"新建文件夹"
                    {//item.Remove();
                        this.listView_Remote.Enabled = false;
                        this.ToolBar_Remote.Enabled = false;
                        this.label_RemoteHint.Text = "正在更新操作，请稍等...";
                        this.CurListViewItem = item;
                        P2PFile_CreateDirCmd(this.GetFullPath(false) + "新建文件夹");
                    }
                }                    
                return;
            }
                        
            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem != null)
                {
                    string srcName = this.GetFullDirPathByDirItem(dItem, false);
                    string decName = this.GetFullPath(false) + e.Label;

                    this.listView_Remote.Enabled = false;
                    this.ToolBar_Remote.Enabled = false;
                    this.label_RemoteHint.Text = "正在更新操作，请稍等...";
                    this.CurListViewItem = item;
                    P2PFile_RenameDirCmd(srcName, decName);                    
                }
                else if (FItem != null)
                {
                    e.CancelEdit = true;
                    this.label_RemoteHint.Text = "暂不支持重命名操作";
                    /*string srcName = this.GetFullFilePathByFileItem(FItem, false);
                    string decName = this.GetFullPath(false) + e.Label;                    

                    this.listView_Remote.Enabled = false;
                    this.ToolBar_Remote.Enabled = false;
                    this.label_RemoteHint.Text = "正在更新操作，请稍等...";
                    this.CurListViewItem = item;
                    P2PFile_RenameFileCmd(srcName, decName);*/
                }
                else if (item.Tag == null)//新建文件夹
                {
                    this.listView_Remote.Enabled = false;
                    this.ToolBar_Remote.Enabled = false;
                    this.label_RemoteHint.Text = "正在更新操作，请稍等...";
                    this.CurListViewItem = item;
                    P2PFile_CreateDirCmd(this.GetFullPath(false) + e.Label);
                }
            }            
        }
        //删除操作(远程)
        private void Btn_RemoteDel_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                if (MessageBox.Show("删除后将不能恢复，请确定是否要进行删除操作.", "提示", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.Cancel)
                    break;

                this.listView_Remote.Enabled = false;
                this.ToolBar_Remote.Enabled = false;
                this.label_RemoteHint.Text = "正在更新操作，请稍等...";

                FileDirInfomation.Ditm dItem = item.Tag as FileDirInfomation.Ditm;
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (dItem != null)
                {
                    string srcName = this.RemoteFileDir.CurDir + "\\" + dItem.dp;
                    this.CurListViewItem = item;
                    P2PFile_DelDirCmd(srcName);
                }
                else if (FItem != null)
                {
                    string srcName = this.RemoteFileDir.CurDir + "\\" + FItem.fp;
                    this.CurListViewItem = item;
                    P2PFile_DelFileCmd(srcName);
                }
            }
        }
        //新建文件夹操作
        private void Btn_RemoteCreateDir_Click(object sender, EventArgs e)
        {
            ListViewItem item = this.listView_Remote.Items.Add("",
                                                              "新建文件夹", 0);
            item.SubItems.Add("");
            item.SubItems.Add("文件夹");
            item.SubItems.Add(DateTime.Now.ToString("yyy-MM-dd hh:mm"));

            item.Tag = null;
            item.Selected = true;
            item.BeginEdit();
        }       
        //刷新远程目录内容
        private void Btn_RemoteRush_Click(object sender, EventArgs e)
        {
            try
            {
                if (!CurDirIsLogDiskRoot(false))
                {
                    this.panel_Right.Enabled = false;
                    this.label_RemoteHint.Enabled = false;
                    this.label_RemoteHint.Text = "正在获取目录内容.请稍等...";
                    P2PFile_GetDirListByPathCmd(this.RemoteFileDir.CurDir);
                }
                else
                    this.Btn_RemoteMainDir_Click(null, new EventArgs());

                this.CurListViewItem = null;
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("获取目录信息出错，请重试!\r\n错误信息:{0}", ex.Message), "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }        
        //改变窗口大小
        private void P2pFileTranslant_Resize(object sender, EventArgs e)
        {
            panel_Left.Width = this.Width / 2-5;
            panel_All_Right.Width = this.Width / 2-5;

            this.TextBox_localPath.Width = this.panel_Left.Width - toolStripLabel2.Width - 10;
            this.TextBox_RemotoPath.Width = this.panel_All_Right.Width - toolStripLabel2.Width - 10;

            this.ToolBar_Remote.AutoSize = false;
            this.ToolBar_Local.AutoSize = false;

            this.ToolBar_Remote.AutoSize = true;
            this.ToolBar_Local.AutoSize = true;
        }
        //发送文件
        private void Btn_LocalSend_Click(object sender, EventArgs e)
        {
            //if (this.CurDirIsDevicesRoot(false))
            {
            //    MessageBox.Show("不能传送到根目录,请在选程机器中选择要选传送的目录再进行发送.", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
            //    return;
            }                        
            
            //this.listView_Local.Enabled = false;
            //this.listView_Remote.Enabled = false;
            //this.ToolBar_Local.Enabled = false;
            //this.ToolBar_Remote.Enabled = false;

            foreach (ListViewItem item in this.listView_Local.SelectedItems)
            {
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (FItem != null)
                {
                    this.CurListViewItem = item;
                    this.P2pFile_SendFileCmd(this.GetFullFilePathByFileItem(FItem, true),this.GetFullPath(false));
                    this.WriteLog(false, "正在发送，请稍等...");
                }
            }
        }
        //接收文件
        private void Btn_RemoteSend_Click(object sender, EventArgs e)
        {
            if (this.CurDirIsDevicesRoot(true))
            {
                MessageBox.Show("不能传送到根目录,请在本地机器中选择要选传送的目录再进行发送.", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }
            
            //this.listView_Local.Enabled = false;
            //this.listView_Remote.Enabled = false;
            //this.ToolBar_Local.Enabled = false;
            //this.ToolBar_Remote.Enabled = false;            

            foreach (ListViewItem item in this.listView_Remote.SelectedItems)
            {
                FileDirInfomation.Fitm FItem = item.Tag as FileDirInfomation.Fitm;
                if (FItem != null)
                {
                    this.CurListViewItem = item;
                    this.P2pFile_RecvFileCmd(this.GetFullFilePathByFileItem(FItem, false), this.GetFullPath(true));
                    this.WriteLog(false, "正在接收，请稍等...");
                }
            }
        }
        //更新界面状态
        private void listView_Local_Click(object sender, EventArgs e)
        {
            listView_Local_SelectedIndexChanged(this.listView_Local, new EventArgs());
            listView_Remote_SelectedIndexChanged(this.listView_Remote, new EventArgs());
        }
        //更新界面状态
        private void listView_Remote_Click(object sender, EventArgs e)
        {
            listView_Local_SelectedIndexChanged(this.listView_Local, new EventArgs());
            listView_Remote_SelectedIndexChanged(this.listView_Remote, new EventArgs());
        }
        //启动文件同步
        private void Btn_connectP2PFileTransLant_Click(object sender, EventArgs e)
        {
            if (this.Btn_connectP2PFileTransLant.Text.Equals("启动文件同步"))
                ConnectDeviceClient();
            else
                ClosedP2pFileTranslant();
        }
        #endregion

        //打开本地路径
        private void TextBox_localPath_DropDownClosed(object sender, EventArgs e)
        {
            try
            {
                if (this.LocalFileDir.CurDir == this.TextBox_localPath.Text)
                    return;

                this.LocalFileDir.CurDir = this.TextBox_localPath.Text;
                if (this.CurDirIsDevicesRoot(true))
                {
                    this.Btn_LocalMainDir_Click(null, new EventArgs());
                }
                else
                {                    
                    if (!string.IsNullOrEmpty(this.LocalFileDir.CurDir))
                        this.UpdateLocalDirListByDirPath();
                }
            }
            catch (Exception ex)
            {
                this.WriteLog(true, string.Format("获取目录内容出错，请重试! 错误信息:{0}", ex.Message));                                               
            }            
        }
        private void TextBox_localPath_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
                TextBox_localPath_DropDownClosed(sender, null);
        }

        //打开远程路径
        private void TextBox_RemotoPath_Enter(object sender, EventArgs e)
        {
            try
            {
                if (this.RemoteFileDir.CurDir == this.TextBox_RemotoPath.Text)
                    return;

                this.panel_Right.Enabled = false;
                this.label_RemoteHint.Enabled = false;
                this.RemoteFileDir.CurDir = this.TextBox_RemotoPath.Text;
                if (this.CurDirIsDevicesRoot(false))
                {
                    this.Btn_RemoteMainDir_Click(null, new EventArgs());
                }
                else
                {                    
                    this.label_RemoteHint.Text = "正在获取目录内容.请稍等...";
                    P2PFile_GetDirListByPathCmd(this.RemoteFileDir.CurDir);
                }
            }
            catch (Exception ex)
            {
                this.WriteLog(false, string.Format("获取目录内容出错，请重试! 错误信息:{0}", ex.Message));
            }
        }

        private void TextBox_RemotoPath_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
                TextBox_RemotoPath_Enter(sender,null);
        }

        private void Btn_CancelOprator_Click_1(object sender, EventArgs e)
        {
            if (!this.Btn_connectP2PFileTransLant.Text.Equals("连接服务器"))
            {
                panel_Right.Enabled = true;
                this.label_RemoteHint.Enabled = true;
            }
        }

        private void MenuItemStopUpOrDown_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.FileUpDownListView.SelectedItems)
            {
                if (!string.IsNullOrEmpty(item.SubItems[6].Text))
                {
                    try
                    {
                        int Handle = System.Convert.ToInt32(item.SubItems[6].Text);
                        this.CurFileServerPtr.stopFileSendOrRecvByHandle(Handle);
                    }
                    catch (System.Exception ex)
                    {
                    	
                    }
                }
            }
        }

        private void FileUpDownListView_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                MenuItemUpDown.Show(Cursor.Position.X, Cursor.Position.Y);
                MenuItemStopUpOrDown.Enabled = false;

                foreach (ListViewItem item in this.FileUpDownListView.SelectedItems)
                    MenuItemStopUpOrDown.Enabled = item.SubItems[4].Text.Contains("%");
            }
        }

        private void ComboBox_Connect_SelectedIndexChanged(object sender, EventArgs e)
        {
            switch(ComboBox_Connect.SelectedIndex)
            {
                case 0://FTP连接方式
                    {
                        //FTP文件服务器
                        CurFileServerPtr = FileServerFactory.CreateFtpFileServer();
                        FTPFileServer FTPPtr = (FTPFileServer)CurFileServerPtr;
                        FTPPtr.OnFWndCallBack += this.FWndCallBackImpl; 

                        Label_Password.Visible = true;
                        Text_UserName.Visible = true;
                        Label_UserName.Visible = true;
                        Text_Password.Visible = true;
                        Text_Port.Text = "21";
                        break;
                    }
                case 1://Socket连接方式
                    {
                        //ACE文件服务器
                        CurFileServerPtr = FileServerFactory.CreateAceFileServer();            
                        ACEFileServer ptr = (ACEFileServer)CurFileServerPtr;
                        ptr.OnFWndCallBack += this.FWndCallBackImpl;

                        Label_Password.Visible = false;
                        Text_UserName.Visible = false;
                        Label_UserName.Visible = false;
                        Text_Password.Visible = false;
                        Text_Port.Text = "19998";
                        break;
                    }
            }//switch
        }


    }        
}
