using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FilePackager.Packaging
{
    static class FNVHash64
    {
        static public UInt64 Compute(string text)
        {
            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            Byte[] bytes = encoding.GetBytes(text);
            return Compute(bytes);
        }

        static public UInt64 Compute(byte[] data)
        {
            // Start with the basis value
            UInt64 hval = 14695981039346656037UL;

            for (int i = 0; i < data.Length; i++)
            {
                // multiply by the FNV magic prime 
                hval *= 1099511628211UL;

                // xor the bottom with the current octet
                hval ^= data[i];
            }

            return hval;
        }
    }
}
