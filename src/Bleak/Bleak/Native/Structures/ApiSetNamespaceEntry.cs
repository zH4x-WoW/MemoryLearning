using System.Runtime.InteropServices;

namespace Bleak.Native.Structures
{
    [StructLayout(LayoutKind.Explicit, Size = 24)]
    internal struct ApiSetNamespaceEntry
    {
        [FieldOffset(0x04)]
        internal readonly int NameOffset;

        [FieldOffset(0x08)]
        internal readonly int NameLength;

        [FieldOffset(0x10)] 
        internal readonly int ValueOffset;
    }
}