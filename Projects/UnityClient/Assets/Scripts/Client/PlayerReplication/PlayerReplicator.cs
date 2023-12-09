using System;
using System.Collections;
using System.Collections.Generic;
using CKPacket;
using Google.Protobuf;
using UnityEngine;
using UnityEngine.UIElements;

public class PlayerReplicator : Replicator
{
    [SerializeField] private ReplicatedPlayerFactory _factory;

    // 수신시 동기화용으로 사용될 패킷을 등록
    protected override IMessage[] GetSynchronizePackets()
        => new IMessage[]
        {
            new resPlayerPosition()
        };

    public override ReplicatedObjectFactory Factory => _factory;

    // replicated 오브젝트 생성 수신 패킷 등록
    // 여기에 등록된 패킷을 수신하면, Factory 클래스의 Generate() 함수 호출
    protected override IMessage GetGenerationTarget()
        => new resReplicatedPlayerGen();

    // replicated 오브젝트 파괴 수신 패킷 등록
    // 여기에 등록된 패킷을 수신하면, Factory 클래스의 Destroy() 함수 호출
    protected override IMessage GetExpireTarget()
        => new resReplicatedPlayerDes();

    private void Awake()
    {
        Init();
    }

    private void OnDestroy()
    {
        Release();
    }
}

[System.Serializable]
public class ReplicatedPlayerFactory : ReplicatedObjectFactory
{
    [SerializeField] private ReplicatedPlayer _prototype;
    
    /// <summary>
    /// 서버에 replicated 객체 생성을 요청.
    /// PlayerReplicator 에서는 사용되지 않음.
    /// </summary>
    public override void NotifyGenerate()
    {
        //Client.TCP.SendPacket(new reqReplicatedPlayerGen());
    }
    /// <summary>
    /// 서버에 replicated 객체 파괴를 요청.
    /// PlayerReplicator 에서는 사용되지 않음.
    /// </summary>
    /// <param name="obj"></param>
    public override void NotifyDestroy(IReplication obj)
    {
        //Client.TCP.SendPacket(new reqReplicatedPlayerDes(){Index = obj.Index});
    }
    /// <summary>
    /// 동기화 패킷을 Recv 했을 때 호출되는 함수.
    /// </summary>
    /// <param name="recvPacket">Replicator 클래스에서 GetSynchronizePackets() 함수로 반환하는 패킷 타입과 같아야함</param>
    /// <returns>해당 패킷에 해당하는 replicated 객체의 인덱스를 반환해야함.</returns>
    public override int GetReplicatedObjectIndexFromRecvHeader(PackedHeader recvPacket)
    {
        return recvPacket.CreateMessage<resPlayerPosition>().Index;
    }
    /// <summary>
    /// 동기화 패킷을 Recv 했을 때 호출되는 함수.
    /// </summary>
    /// <param name="recvPacket">Replicator 클래스에서 GetExpireTarget() 함수로 반환하는 패킷 타입과 같아야함</param>
    /// <returns>해당 패킷에 해당하는 replicated 객체의 인덱스를 반환해야함.</returns>
    public override int GetReplicatedObjectIndexFromDesHeader(PackedHeader expiringPacket)
    {
        return expiringPacket.CreateMessage<resReplicatedPlayerDes>().Index;
    }

    /// <summary>
    /// 객체의 변화를 동기화 시키고 싶을 때 사용.
    /// 여기서는 local 플레이어 객체를 동기화하려고 사용함.
    /// </summary>
    /// <param name="obj">동기화 대상인 replicated 객체</param>
    public override void NotifySynchronize(IReplication obj)
    {
        if (obj is not ReplicatedPlayer p) return;

        var position = p.transform.position;
        var packet = new reqPlayerPosition()
        {
            Index = obj.Index,
            X = position.x,
            Y = position.y,
            Z = position.z
        };
        
        
        Client.TCP.SendPacket(packet);
    }

    /// <summary>
    /// Recv 패킷을 수신했을 때 처리
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="recvPacket"></param>
    protected override void RecvSynchronize(IReplication obj, PackedHeader recvPacket)
    {
        if (obj is not ReplicatedPlayer p) return;
        var pos = recvPacket.CreateMessage<resPlayerPosition>();

        p.transform.position = new Vector3(pos.X, pos.Y, pos.Z);
    }

    /// <summary>
    /// 생성 요청을 받았을 때 호출되는 함수
    /// </summary>
    /// <param name="generatingPacket">Replicator 클래스의 GetGenerationTarget() 함수의 반환값과 같아야함.</param>
    /// <returns>요청에 의해 생성된 replicated 객체</returns>
    protected override IReplication Generate(PackedHeader generatingPacket)
    {
        var packet = generatingPacket.CreateMessage<resReplicatedPlayerGen>();

        var player = GameObject.Instantiate(_prototype);
        player.Index = packet.Index;

        player.name = $"replicated_player_{player.Index}";
        
        return player;
    }

    protected override void Destroy(IReplication obj)
    {
        if(obj is ReplicatedPlayer p)
            Destroy(p);
    }

}
