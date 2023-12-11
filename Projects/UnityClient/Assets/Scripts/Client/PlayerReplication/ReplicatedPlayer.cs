using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ReplicatedPlayer : MonoBehaviour, IReplication
{
    [SerializeField] private PlayerReplicator _replicator;
    [SerializeField] private bool _isLocal;
    [SerializeField] private int _index;
    [SerializeField] private float _speed;

    public Replicator Replicator
    {
        get => _replicator;
        set => _replicator = value as PlayerReplicator;
    }

    public int Index
    {
        get => _index;
        set => _index = value;
    }
    
    public float R { get; set; }
    public float G { get; set; }
    public float B { get; set; }
    public float A { get; set; }
    
    public void OnInit()
    {
    }

    public void OnExpired()
    {
        
    }

    private void Awake()
    {
        if (_isLocal == false) return;
        
        if (this.Replicator.Factory is ReplicatedPlayerFactory factory)
        {
            factory.NotifyGenerate();
        }
        this.Replicator.AddReplicatedObject(this);
    }

    private void Update()
    {
        if (this.Replicator == null) return;
        if (_isLocal == false) return;
        Index = (int)Client.Index;

        float x = Input.GetAxis("Horizontal");
        float z = Input.GetAxis("Vertical");

        var pos = new Vector3(x, 0f, z);
        transform.position += pos * Time.deltaTime * _speed;

        if (x == 0 && z == 0)
            return;
        
        if (this.Replicator.Factory is ReplicatedPlayerFactory factory)
        {
            factory.NotifySynchronize(this);
        }
    }

    private void OnCollisionEnter(Collision collision)
    {
        if (this.Replicator == null) return;
        
        if (collision.transform.CompareTag("Color"))
        {
            print("hit");
            R = collision.gameObject.GetComponent<MeshRenderer>().material.color.r;
            G = collision.gameObject.GetComponent<MeshRenderer>().material.color.g;
            B = collision.gameObject.GetComponent<MeshRenderer>().material.color.b;
            A = collision.gameObject.GetComponent<MeshRenderer>().material.color.a;
        }
        
        if (this.Replicator.Factory is ReplicatedPlayerFactory factory)
        {
            factory.NotifySynchronize(this);
        }
    }
}
