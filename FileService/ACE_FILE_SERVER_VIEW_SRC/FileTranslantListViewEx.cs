using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;

namespace ACE_FILE_SERVER_VIEW
{
    public partial class FileTranslantListViewEx : ListView
    {
        public FileTranslantListViewEx(IContainer container)
        {
            container.Add(this);

            InitializeComponent();

            SetStyle(ControlStyles.DoubleBuffer | ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint, true);
            UpdateStyles();  
        } 
    }
}
