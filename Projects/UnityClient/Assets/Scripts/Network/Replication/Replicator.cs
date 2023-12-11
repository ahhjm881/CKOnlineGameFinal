using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Runtime.CompilerServices;
using CKPacket;
using Google.Protobuf;
using JetBrains.Annotations;
using UnityEngine;

public abstract class Replicator : MonoBehaviour
{
    private List<IReplication> _list = new();
    
    // replication 이벤트를 발생시킬 패킷. 이 패킷이 수신되면 replicate 함
    protected abstract Google.Protobuf.IMessage GetGenerationTarget();
    protected abstract Google.Protobuf.IMessage GetExpireTarget();
    protected abstract Google.Protobuf.IMessage[] GetSynchronizePackets();
    public abstract ReplicatedObjectFactory Factory { get; }
    protected void Init()
    {
        Factory._replicator = this;
        
        Client.TCP.AddPacketReceiver(CreateReplicatedObject, GetGenerationTarget().Descriptor.FullName);
        Client.TCP.AddPacketReceiver(ExpireReplicatedObject, GetExpireTarget().Descriptor.FullName);

        foreach (var packet in GetSynchronizePackets())
        {
            Client.TCP.AddPacketReceiver(RecvSynchronize, packet.Descriptor.FullName);
        }
    }

    protected void Release()
    {
        Client.TCP.RemovePacketReceiver(CreateReplicatedObject, GetGenerationTarget().Descriptor.FullName);
        Client.TCP.RemovePacketReceiver(ExpireReplicatedObject, GetExpireTarget().Descriptor.FullName);
        foreach (var packet in GetSynchronizePackets())
        {
            Client.TCP.RemovePacketReceiver(RecvSynchronize, packet.Descriptor.FullName);
        }
    }

    private void RecvSynchronize(PackedHeader header)
    {
        var obj = GetReplicatedObjectFromRecvHeader(header);

        if (obj == null) return;
        Factory.Internal_RecvSynchronize(obj, header);
    }
    public void AddReplicatedObject(IReplication obj)
    {
        _list.Add(obj);
        obj.Replicator = this;
        obj.OnInit();
    }

    public bool RemoveReplicatedObject(IReplication obj)
    {
        obj.OnExpired();
        return _list.Remove(obj);
    }

    private void CreateReplicatedObject(CKPacket.PackedHeader header)
    {
        Debug.Log("gen c");
        var obj = Factory.Internal_Generate(header);
        if (obj == null) return;
        AddReplicatedObject(obj);
    }

    private void ExpireReplicatedObject(CKPacket.PackedHeader header)
    {
        var obj = GetReplicatedObjectFromDesHeader(header);
        Factory.Internal_Destroy(obj);

        if (obj == null)
        {
            Debug.LogError("Replicated Object expire 실패");
        }
        if (RemoveReplicatedObject(obj) == false)
        {
            Debug.LogError("Replicated Object expire 실패");
        }
    }

    [CanBeNull]
    private IReplication GetReplicatedObjectFromRecvHeader(PackedHeader header)
    {
        int index = Factory.GetReplicatedObjectIndexFromRecvHeader(header);

        return GetReplicatedObjectFromIndex(index);
    }
    [CanBeNull]
    private IReplication GetReplicatedObjectFromDesHeader(PackedHeader header)
    {
        int index = Factory.GetReplicatedObjectIndexFromDesHeader(header);

        return GetReplicatedObjectFromIndex(index);
    }

    private IReplication GetReplicatedObjectFromIndex(int index)
    {
        if (index == -1) 
        {
            //Debug.LogError("Replicator: 유효하지 않는 index: " + index);
            return null;
        }

        var obj = _list.Find(x=>x.Index == index);
        if (obj == null)
        {
            //Debug.LogError("Replicator: 유효하지 않는 index: " + index);
            return null;
        }

        return obj;
    }
}

public abstract class ReplicatedObjectFactory
{
    internal Replicator _replicator;
    public abstract int GetReplicatedObjectIndexFromRecvHeader(CKPacket.PackedHeader recvPacket);
    public abstract int GetReplicatedObjectIndexFromDesHeader(CKPacket.PackedHeader expiringPacket);
    public abstract void NotifyGenerate();
    public abstract void NotifyDestroy(IReplication obj);
    public abstract void NotifySynchronize(IReplication obj);
    
    [CanBeNull]
    internal void Internal_RecvSynchronize(IReplication obj, CKPacket.PackedHeader recvPacket)
    {
        RecvSynchronize(obj, recvPacket);
    }

    internal void Internal_Destroy(IReplication obj)
    {
        Destroy(obj);
    }

    [CanBeNull]
    internal IReplication Internal_Generate(CKPacket.PackedHeader generatingPacket)
    {
        return Generate(generatingPacket);
    }

    [CanBeNull] protected abstract void RecvSynchronize(IReplication obj, CKPacket.PackedHeader recvPacket);
    [CanBeNull] protected abstract IReplication Generate(CKPacket.PackedHeader generatingPacket);
    protected abstract void Destroy(IReplication obj);
}
