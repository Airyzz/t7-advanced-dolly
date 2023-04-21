using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CSNamedPipe;

namespace adc_debug
{
    class Program
    {
        static void Main(string[] args)
        {

            NamedPipeServer PServer = new NamedPipeServer(@"\\.\pipe\T7xADC", 0);
            Console.WriteLine("Starting Communication... " + PServer.pipeName);

            Console.WriteLine("Done!");

            PServer.Start();

            while(Console.ReadLine() != "quit")
            {
                //do nothing
            };

            PServer.StopServer();
        }
    }
}
