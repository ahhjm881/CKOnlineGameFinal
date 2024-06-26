﻿using System.Net.Sockets;
using CKPacket;
using UnityEngine;

public static class Client
{
    private const string IP = "127.0.0.1";
    //public static NetworkClient TCP = new NetworkClient(IP, 5004, ProtocolType.Tcp);
    public static NetworkClient TCP ;
    public static NetworkClient UDP ;

    public static string Name { get; private set; }

    public static uint Index { get; private set; }

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSplashScreen)]
    private static void Init()
    {
        TCP = new NetworkClient(IP, 11022, ProtocolType.Tcp);
        UDP = new NetworkClient(IP, 5025, ProtocolType.Udp);
    }

    public static void Start()
    {
        TCP.Start();
        UDP.Start();
        TCP.OnDisconnect += OnDisconnect;
        Application.wantsToQuit += OnApplicationQuit;

        TCP.AddPacketReceiver(Join, resPlayerJoin.Descriptor.FullName);
    }

    private static void Join(PackedHeader header)
    {
        var join = header.CreateMessage<resPlayerJoin>();
        Index = header.MessageHeader.UserId;
        Debug.Log("server joined, index: " + Index);
        
        //UnityEngine.Random.InitState((int)Index);
        //string n = "";
        //for (int i = 0; i < 10; i++)
        //{
        //    n += (char)(Random.Range(0, 26) + 65);
        //}
//
        //Name = n;
        Name = "Test";
        
        Debug.Log(Name);
        
        TCP.SendPacket(new reqNotifyPlayerName()
        {
            Name = Name
        });
    }

    private static bool OnApplicationQuit()
    {
        Close();
        return true;
    }

    private static void OnDisconnect()
    {
        TCP.RemovePacketReceiver(Join, resPlayerJoin.Descriptor.FullName);
    }

    public static void Close()
    {
        if (TCP != null)
        {
            TCP.Close();
        }
        if (UDP != null)
        {
            UDP.Close();
        }
    }
}
