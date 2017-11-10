using System;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Drawing;


namespace ACE_FILE_SERVER_VIEW
{
    /*************************************************************************************
            * 说明:本地目录的信息
            * 建立日期:2012-8
            * 更新日期:2012-8
            * 建立者:夏
            * 备注:记录了当前所有的工程师列表和服务台列表
            * ************************************************************************************/
    public class FileDirInfomation
    {
        public List<Ditm> DirLists = new List<Ditm>();
        public List<Fitm> FileLists = new List<Fitm>();
        public List<Device> Devices = new List<Device>();
        public string CurDir = "";


        #region 子类信息
        public class Ditm
        {
            public string dp { get; set; }
            public string ct { get; set; }
            public Ditm()
            {
            }
            public Ditm(string DirPath, string changeTime)
            {
                this.dp = DirPath;
                this.ct = changeTime;
            }
        }
        public class Fitm
        {
            public string fp { get; set; }
            public string size { get; set; }
            public string ct { get; set; }
            public Fitm()
            {
            }
            public Fitm(string FilePath, string FileSize, string changeTime)
            {
                this.fp = FilePath;
                this.size = FileSize;
                this.ct = changeTime;
            }
            public string GetTypeStr()
            {
                return P2pFileTransLantHelper.GetTypeName(this.fp);
            }
            public Icon GetIcon()
            {
                return P2pFileTransLantHelper.GetIcon(this.fp, false);
            }
        }
        public class Device
        {
            public string name { get; set; }
            public string type { get; set; }
            public Device()
            {
            }
            public Device(string name, string type)
            {
                this.name = name;
                this.type = type;
            }
        }
        #endregion
    }



    /// <summary>
    /// 保存文件信息的结构体
    /// </summary>
    /// 
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public struct SHFILEINFO
    {
        public IntPtr hIcon;
        public int iIcon;
        public uint dwAttributes;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
        public string szDisplayName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
        public string szTypeName;
    }

    /*************************************************************************************
    * 说明:获取本地文件扩展信息
    * 建立日期:2012-8
    * 更新日期:2012-8
    * 建立者:夏
    * 备注:
    * ************************************************************************************/
    public static class P2pFileTransLantHelper
    {

        [DllImport("Shell32.dll", EntryPoint = "SHGetFileInfo", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr SHGetFileInfo(string pszPath, uint dwFileAttributes, ref SHFILEINFO psfi, uint cbFileInfo, uint uFlags);

        [DllImport("User32.dll", EntryPoint = "DestroyIcon")]
        public static extern int DestroyIcon(IntPtr hIcon);

        #region API 参数的常量定义
        public const uint SHGFI_ICON = 0x100;
        public const uint SHGFI_LARGEICON = 0x0; //大图标 32×32
        public const uint SHGFI_SMALLICON = 0x1; //小图标 16×16
        public const uint SHGFI_USEFILEATTRIBUTES = 0x000000010;
        public const uint SHGFI_TYPENAME = 0x000000400;
        public const uint FILE_ATTRIBUTE_NORMAL = 0x00000080;
        #endregion

        /// <summary>
        /// 获取文件类型的关联图标/文件夹图标
        /// </summary>
        /// <param name="fileName">文件类型的扩展名或文件的绝对路径</param>
        /// <param name="isLargeIcon">是否返回大图标</param>
        /// <returns>获取到的图标</returns>
        public static Icon GetIcon(string fileName, bool isLargeIcon)
        {
            SHFILEINFO shfi = new SHFILEINFO();
            IntPtr hI;
            Icon icon = null;

            if (isLargeIcon)
                hI = SHGetFileInfo(fileName, 0, ref shfi, (uint)Marshal.SizeOf(shfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
            else
                hI = SHGetFileInfo(fileName, 0, ref shfi, (uint)Marshal.SizeOf(shfi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

            if (shfi.hIcon != IntPtr.Zero)
            {
                icon = Icon.FromHandle(shfi.hIcon).Clone() as Icon;
                DestroyIcon(shfi.hIcon); //释放资源
            }
            return icon;
        }

        public static string GetTypeName(string fileName)
        {
            SHFILEINFO fileInfo = new SHFILEINFO();  //初始化FileInfomation结构

            //调用GetFileInfo函数，最后一个参数说明获取的是文件类型(SHGFI_TYPENAME)
            IntPtr res = SHGetFileInfo(fileName,
                                     (uint)FILE_ATTRIBUTE_NORMAL,
                                     ref fileInfo,
                                     (uint)Marshal.SizeOf(fileInfo),
                                     (uint)SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

            return fileInfo.szTypeName;
        }
    }


    /*************************************************************************************
    * 说明:类
    * 建立日期:2012-8
    * 更新日期:2012-8
    * 建立者:夏
    * 备注:FileDirHelper.Dll方法，有时多次调用会报异常，这里还是用本地代码替换
    * ************************************************************************************/
    public static class FileDirHelper
    {
        public static string GetDeviceListXml()
        {
            DriveInfo[] allDrives = DriveInfo.GetDrives();
            string ret = "";

            ret = "<DeviceList>";
            foreach (DriveInfo d in allDrives)
            {
                ret += string.Format("<Device><name>{0}</name><type>", d.Name);
                switch (d.DriveType)
                {
                    case DriveType.CDRom: ret += "CD 驱动器"; break;
                    case DriveType.Fixed: ret += "本地磁盘"; break;
                    case DriveType.Network: ret += "网络磁盘"; break;
                    case DriveType.Ram: ret += "虚拟磁盘"; break;
                    case DriveType.Removable: ret += "可移动磁盘"; break;
                    case DriveType.Unknown: ret += "未知设备:"; break;
                    default: ret += "系统设备"; break;
                }
                ret += "</type></Device>";
            }
            ret += "</DeviceList>";
            return ret;
        }
        //获取本地目录和文件列表，得到后直接通过GetDirListByPathMsg发送(这里的异常没有拦截，交给界面层去处理)
        public static string GetDirListByPathXml(string path)
        {
            string szFile = "";
            if (string.IsNullOrEmpty(path) || path.Equals("\\"))
                return "";

            szFile = "<DFList>"; 
            DirectoryInfo TheFolder = new DirectoryInfo(path);
            if (TheFolder != null)
            {
                szFile += "<DList>";
                foreach (DirectoryInfo NextFolder in TheFolder.GetDirectories())
                    szFile += string.Format("<Ditm><dp>{0}</dp><ct>{1}</ct></Ditm>", NextFolder.Name, NextFolder.LastWriteTime.ToString("yyy-MM-dd hh:mm"));
                szFile += "</DList>";

                szFile += "<FList>";
                foreach (FileInfo NextFile in TheFolder.GetFiles())
                {
                    long size = NextFile.Length;
                    long fileSize = size / (long)1024 / (long)1024;
                    string pzSize = "";
                    if (fileSize > (long)1.0)
                        pzSize = string.Format("{0}MB", fileSize);
                    else
                        pzSize = string.Format("{0}KB", size / (long)1024 == 0 ? 1 : size / (long)1024);

                    szFile += string.Format("<Fitm><fp>{0}</fp><size>{1}</size><ct>{2}</ct></Fitm>", NextFile.Name, pzSize, NextFile.LastWriteTime.ToString("yyy-MM-dd hh:mm"));
                }
                szFile += "</FList>";
            }
            szFile += "</DFList>";

            return szFile;
        }

        public static bool CreateOrDelDir(int operatorType, string DirPathName)
        {
            if (string.IsNullOrEmpty(DirPathName))
                return false;

            try
            {
                switch (operatorType)
                {
                    case 0://创建目录
                        {
                            if (Directory.Exists(DirPathName))
                                return false;
                            else
                            {
                                DirectoryInfo TheFolder = Directory.CreateDirectory(DirPathName);
                                return (TheFolder != null);
                            }
                        }
                    case 1://删除目录
                        {
                            Directory.Delete(DirPathName, true);
                            return true;
                        }
                    default:
                        return false;
                }
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public static bool DelFile(string filePath)
        {
            try
            {
                if (!File.Exists(filePath))
                    return false;

                File.Delete(filePath);
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public static bool RenameDirOrFile(int operatorType, string srcDirOrFileName, string DesDirOrFileName)
        {
            try
            {
                switch (operatorType)
                {
                    case 0://0文件改名
                        {
                            if (File.Exists(DesDirOrFileName))
                                return false;
                            else
                                File.Move(srcDirOrFileName, DesDirOrFileName);
                            return true;
                        }
                    case 1://1目录改名
                        {
                            if (Directory.Exists(DesDirOrFileName))
                                return false;
                            Directory.Move(srcDirOrFileName, DesDirOrFileName);
                            return true;
                        }
                    default:
                        return false;
                }
            }
            catch (Exception ex)
            {
                string log = string.Format("{0},{1}", ex.Message, ex.Source);
                return false;
            }
        }
    }
}