using System.Collections;
using System.Collections.Generic;
using CKPacket;
using UnityEngine;

public interface IPacketReceiver
{
    void OnPacketReceived(PackedHeader packet);
}
