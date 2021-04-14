using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace rtspplayertest
{
    public partial class Form1 : Form
    {
        public delegate void logFunc(int level, string msg);
        private SWIGTYPE_p__gmf_player m_player;
        public Form1()
        {
            InitializeComponent();

            MediaCore.gmf_init(0, 0, "");

            logFunc func = new logFunc(logPrint);
            MediaCore.gmf_set_log_func(new SWIGTYPE_p_f_int_p_q_const__char__void(Marshal.GetFunctionPointerForDelegate(func), false));
        }

        ~Form1()
        {
            MediaCore.gmf_uninit();
        }

        private void logPrint(int level, string msg)
        {
            Console.WriteLine(msg);
        }

        private void button_play_Click(object sender, EventArgs e)
        {
            m_player = MediaCore.gmf_player_create("player1", "");
            MediaCore.gmf_player_open(m_player, this.textBox_url.Text, new SWIGTYPE_p_void(this.display.Handle, false));
            MediaCore.gmf_player_play(m_player);
        }

        private void button_stop_Click(object sender, EventArgs e)
        {
            MediaCore.gmf_player_close(m_player);
            MediaCore.gmf_player_destory(m_player);
        }
    }
}
