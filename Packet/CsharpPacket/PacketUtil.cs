using System;
using System.IO;
using Google.Protobuf;
using Google.Protobuf.Reflection;

namespace CKPacket
{
    public class PackedHeader
    {
        private messageHeader _messageHeader;
        public string MessagType => _messageHeader.Type;
        public messageHeader MessageHeader => _messageHeader;
        public PackedHeader(messageHeader messageHeader) { _messageHeader = messageHeader; }

        public T CreateMessage<T>() where T : class, IMessage, new()
        {
            T message = new T();
            message.MergeFrom(_messageHeader.Arr.ToByteArray(), 0, (int)_messageHeader.ArrSize);

            return message;
        }
    }

    public static class PacketUtil
    {
        public static byte[] Int32ToByteArray(int value)
        {
            byte[] bytes = new byte[4];
            bytes[0] = (byte)((value >> 24) & 0xFF);
            bytes[1] = (byte)((value >> 16) & 0xFF);
            bytes[2] = (byte)((value >> 8) & 0xFF);
            bytes[3] = (byte)(value & 0xFF);
            return bytes;
        }

        public static int ByteArrayToInt32(byte[] bytes)
        {
            return (bytes[0] << 24) |
                   (bytes[1] << 16) |
                   (bytes[2] << 8) |
                   bytes[3];
        }

        public static bool Pack(
            IMessage message,
            out byte[] header
            )
        {
            header = null;
            if (message == null) return false;

            var arr = message.ToByteString();

            messageHeader messageHeader = new messageHeader();
            messageHeader.Arr = arr;
            messageHeader.ArrSize = (uint)arr.Length;
            messageHeader.Type = message.Descriptor.FullName;

            byte[] msgBuf = messageHeader.ToByteArray();
            const int TYPE_SIZE = 4;

            header = new byte[msgBuf.Length + TYPE_SIZE];

            Array.Copy(Int32ToByteArray(msgBuf.Length), header, TYPE_SIZE);
            Array.Copy(msgBuf, 0, header, TYPE_SIZE, msgBuf.Length);

            return true;
        }

        public static bool Unpack(
            byte[] buffer,
            out PackedHeader messageHeader
            )
        {
            messageHeader = null;
            if (buffer == null) return false;

            var header = new messageHeader();
            int len = ByteArrayToInt32(buffer);

            const int TYPE_SIZE = 4;

            header.MergeFrom(buffer, TYPE_SIZE, len);

            messageHeader = new PackedHeader(header);

            return true;
        }
    }
}
