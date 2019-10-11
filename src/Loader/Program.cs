using Bleak;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Loader
{
    class Program
    {
        static void Main(string[] args)
        {
            ProcessStartInfo processStartInfo = new ProcessStartInfo(@"D:\Coding\MemoryLearning\test-exe\wow.exe");
            Process process = Process.Start(processStartInfo);
            Thread.Sleep(100);

            using (var injector = new Injector("Wow", @"D:\Coding\MemoryLearning\src\x64\Debug\InjectMeDLL.dll", InjectionMethod.CreateThread, InjectionFlags.HideDllFromPeb | InjectionFlags.RandomiseDllHeaders | InjectionFlags.RandomiseDllName))
            {
                // Inject the DLL into the remote process
                var dllBaseAddress = injector.InjectDll();
                Console.WriteLine($"DLL Injected its base address is: 0x{dllBaseAddress:X}");

                Console.WriteLine("Press <Enter> key to eject DLL...");
                // Eject the DLL from the process
                Console.ReadLine();

                injector.EjectDll();
                Console.WriteLine("Dll Ejected.");

                Console.ReadLine();
            }
        }
    }
}
