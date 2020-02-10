using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace UiPathTeam.KeyboardExtension.Activities
{
    internal static class Bridge
    {
        public const int BLOCK_KEYBD = 1 << 0;
        public const int BLOCK_MOUSE = 1 << 1;
        public const int RESET_IME = 1 << 0;
        public const int SET_IME = 1 << 1;

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        public static extern IntPtr LoadLibraryW(string dllToLoad);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);

        private static readonly string DLLNAME = string.Format("UiPathTeam.KeyboardExtension{0}.dll", IntPtr.Size > 4 ? 64 : 32);
        private static IntPtr _dllPtr;

        static Bridge()
        {
            string path = Path.Combine(Path.GetDirectoryName(Assembly.GetAssembly(typeof(Bridge)).Location), DLLNAME);
            _dllPtr = LoadLibraryW(path);
            if (_dllPtr == (IntPtr)null)
            {
                throw new Exception(string.Format("Unable to load {0}.", path));
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate int UnmanagedGetBlockInput();

        public static int GetBlockInput()
        {
            return ((UnmanagedGetBlockInput)GetDelegate("GetBlockInput", typeof(UnmanagedGetBlockInput)))();
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate int UnmanagedGetFlags(IntPtr hwnd);

        public static int GetFlags(IntPtr hwnd)
        {
            return ((UnmanagedGetFlags)GetDelegate("GetFlags", typeof(UnmanagedGetFlags)))(hwnd);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate ushort UnmanagedGetPreferredKeyboardLayout();

        public static ushort GetPreferredKeyboardLayout()
        {
            return ((UnmanagedGetPreferredKeyboardLayout)GetDelegate("GetPreferredKeyboardLayout", typeof(UnmanagedGetPreferredKeyboardLayout)))();
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate int UnmanagedSetBlockInput(int flags);

        public static int SetBlockInput(int flags)
        {
            return ((UnmanagedSetBlockInput)GetDelegate("SetBlockInput", typeof(UnmanagedSetBlockInput)))(flags);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate int UnmanagedSetFlags(IntPtr hwnd, int flags);

        public static int SetFlags(IntPtr hwnd, int flags)
        {
            return ((UnmanagedSetFlags)GetDelegate("SetFlags", typeof(UnmanagedSetFlags)))(hwnd, flags);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void UnmanagedSetToggleSequence(string seq);

        public static void SetToggleSequence(string seq)
        {
            ((UnmanagedSetToggleSequence)GetDelegate("SetToggleSequence", typeof(UnmanagedSetToggleSequence)))(seq);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void UnmanagedSetPreferredKeyboardLayout(ushort langId);

        public static void SetPreferredKeyboardLayout(ushort langId)
        {
            ((UnmanagedSetPreferredKeyboardLayout)GetDelegate("SetPreferredKeyboardLayout", typeof(UnmanagedSetPreferredKeyboardLayout)))(langId);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate bool UnmanagedStartAgent(IntPtr hwnd);

        public static bool StartAgent(IntPtr hwnd)
        {
            return ((UnmanagedStartAgent)GetDelegate("StartAgent", typeof(UnmanagedStartAgent)))(hwnd);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void UnmanagedStartServer();

        public static void StartServer()
        {
            ((UnmanagedStartServer)GetDelegate("StartServer", typeof(UnmanagedStartServer)))();
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void UnmanagedStopAgent(IntPtr hwnd);

        public static void StopAgent(IntPtr hwnd)
        {
            ((UnmanagedStopAgent)GetDelegate("StopAgent", typeof(UnmanagedStopAgent)))(hwnd);
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void UnmanagedStopServer();

        public static void StopServer()
        {
            ((UnmanagedStopServer)GetDelegate("StopServer", typeof(UnmanagedStopServer)))();
        }

        private static Delegate GetDelegate(string name, Type type)
        {
            var ptr = GetProcAddress(_dllPtr, name);
            if (ptr == (IntPtr)null)
            {
                throw new Exception(string.Format("Unable to find {0}.", name));
            }
            return Marshal.GetDelegateForFunctionPointer(ptr, type);
        }

        /// <summary>
        /// Accepts not only an immediate Win32 handle but also UiPath.Core.Window and UiPath.Core.UiElement
        /// and returns an IntPtr representation of Win32 window handle.
        /// </summary>
        /// <param name="value">Either Win32 window handle, Window, or UiElement</param>
        /// <returns>IntPtr representation of Win32 window handle</returns>
        public static IntPtr ToWin32Handle(object value)
        {
            var type = value.GetType();
            if (type == typeof(IntPtr))
            {
                return (IntPtr)value;
            }
            else if (type == typeof(int))
            {
                return (IntPtr)(int)value;
            }
            else if (type == typeof(long))
            {
                return (IntPtr)(long)value;
            }
            else if (type.Namespace == "UiPath.Core")
            {
                // to eliminate dependency...
                if (type.Name == "Window")
                {
                    var pi = type.GetProperty("Handle");
                    if (pi == null)
                    {
                        throw new InvalidCastException(type.FullName + " has no Handle property.");
                    }
                    var h = (IntPtr)pi.GetValue(value);
                    if (h == (IntPtr)0)
                    {
                        throw new InvalidCastException(type.FullName + ".Handle has no value.");
                    }
                    return h;
                }
                else if (type.Name == "UiElement")
                {
                    var mi = type.GetMethod("Get");
                    if (mi == null)
                    {
                        throw new InvalidCastException(type.FullName + " has no Get method.");
                    }
                    var h = (IntPtr)(int)mi.Invoke(value, new object[] { "hwnd", false });
                    if (h == (IntPtr)0)
                    {
                        throw new InvalidCastException(type.FullName + ".Get(\"hwnd\",false) returns no value.");
                    }
                    return h;
                }
                else if (type.Name == "Browser")
                {
                    var pi = type.GetProperty("Element");
                    if (pi == null)
                    {
                        throw new InvalidCastException(type.FullName + " has no Element property.");
                    }
                    var element = pi.GetValue(value);
                    if (element == null)
                    {
                        throw new InvalidCastException(type.FullName + ".Element has no value.");
                    }
                    return ToWin32Handle(element);
                }
            }
            throw new InvalidCastException(type.FullName + " is not supported.");
        }
    }
}
