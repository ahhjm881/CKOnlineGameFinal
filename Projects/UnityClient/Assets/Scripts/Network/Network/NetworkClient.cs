using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using CKPacket;
using Google.Protobuf;
using UnityEngine;
public class NetworkClient
{
    private const int UDP_MAX_DATA_LENGTH = 512;
    private Socket socket;
    private readonly IPEndPoint endPoint;
    private readonly ProtocolType socketProtocol;
    private readonly SynchronizationContext synchronizationContext;
    private readonly Dictionary<string, List<System.Action<CKPacket.PackedHeader>>> packetReceivers = new();

    public event Action OnDisconnect;

    public NetworkClient(string ip, int port, ProtocolType protocol)
    {
        endPoint = new IPEndPoint(IPAddress.Parse(ip), port);
        socket = new Socket(endPoint.AddressFamily, (protocol == ProtocolType.Udp) ? SocketType.Dgram : SocketType.Stream, protocol);
        socketProtocol = protocol;
        synchronizationContext = SynchronizationContext.Current;
    }

    public void Start()
    {
        socket.Connect(endPoint);
        if (socketProtocol == ProtocolType.Tcp)
        {
            new Thread(ReadTcpDataThread).Start();
        }
        else if (socketProtocol == ProtocolType.Udp)
        {
            new Thread(ReadUdpDataThread).Start();
        }
    }

    private void ReadUdpDataThread()
    {
        //byte[] clientBuffer = new byte[UDP_MAX_DATA_LENGTH];
        //EndPoint ep = socket.RemoteEndPoint;
        //while (socket != null)
        //{
        //    int bytesReceived = 0;
        //    try
        //    {
        //        bytesReceived = socket.ReceiveFrom(clientBuffer, 0, UDP_MAX_DATA_LENGTH, SocketFlags.None, ref ep);
        //    }
        //    catch
        //    {
        //        break;
        //    }
        //    if (bytesReceived > 0)
        //    {
        //        PacketBase packetBase = default;
        //        Packet packet = default;
        //        packetBase.packet_id = BitConverter.ToUInt16(clientBuffer, 0);
        //        packetBase.length = BitConverter.ToUInt16(clientBuffer, sizeof(ushort));
        //        packet.pbase = packetBase;
        //        packet.data = UnsafeCode.SubArray(clientBuffer, sizeof(PacketBase), packetBase.length - sizeof(PacketBase));
        //        synchronizationContext.Post((object state) => { HandlePacket(packet); }, null);
        //    }
        //    else if (bytesReceived < 0)
        //    {
        //        break;
        //    }
        //    Thread.Sleep(50);
        //}
        //Close();
    }

    private void ReadTcpDataThread()
    {
        byte[] buf = new Byte[4096];
        PackedHeader header;
        while (socket != null)
        {
            int recvByte = socket.Receive(buf);

            // connection closed
            if (recvByte == 0)
            {
                break;
            }

            var tempBuf = new byte[recvByte];
            Array.Copy(buf, tempBuf, recvByte);
            if (PacketUtil.Unpack(tempBuf, out header) == false)
            {
                // TODO: insert error code
                continue;
            }

            var header1 = header;
            synchronizationContext.Post((object state) => { HandlePacket(header1); }, null);
            
            // reset
            header = null;
        }
        Close();
        if (OnDisconnect != null)
        {
            synchronizationContext.Post((object state) => { OnDisconnect(); }, null);
        }
    }



    public void HandlePacket(PackedHeader packet)
    {
        if (packetReceivers.TryGetValue(packet.MessagType, out var list))
        {
            for (int i = 0; i < list.Count; i++)
            {
                list[i]?.Invoke(packet);
            }
        }

    }

    private void SendData(byte[] data)
    {
        if (socket != null)
        {
            try
            {
                if (socketProtocol == ProtocolType.Tcp)
                {
                    socket.Send(data);
                }
                else
                {
                    socket.SendTo(data, endPoint);
                }
            }
            catch { Close(); }
        }
    }

    public void SendPacket(IMessage packet)
    {
        if (packet == null)
        {
            Debug.LogError("Packet이 null 입니다.");
            return;
        }
        
        byte[] arr;
        if (PacketUtil.Pack(packet, out arr) == false)
        {
            // TODO: insert error code
            Debug.Log("NetworkClient packing error");
            return;
        }
        
        SendData(arr);
    }

    public NetworkClient AddPacketReceiver(System.Action<PackedHeader> callback, string packetType)
    {
        if (packetReceivers.TryGetValue(packetType, out var list) == false)
        {
            packetReceivers.Add(packetType, list = new List<System.Action<PackedHeader>>());
        }
        
        list.Add(callback);

        return this;
    }

    public NetworkClient RemovePacketReceiver(System.Action<PackedHeader> callback, string packetType)
    {
        if (packetReceivers.TryGetValue(packetType, out var list))
        {
            list.Remove(callback);
        }

        return this;
    }

    public void Close()
    {
        if (socket != null)
        {
            socket.Dispose();
            socket = null;
        }
    }
}
