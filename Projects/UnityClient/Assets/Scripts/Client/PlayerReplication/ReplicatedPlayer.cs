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

    public int Index
    {
        get => _index;
        set => _index = value;
    }

    public void OnInit()
    {
    }

    public void OnExpired()
    {
        
    }

    private void Awake()
    {
        if (_isLocal == false) return;
        
        if (_replicator.Factory is ReplicatedPlayerFactory factory)
        {
            factory.NotifyGenerate();
        }
    }

    private void Update()
    {
        if (_isLocal == false) return;

        float x = Input.GetAxis("Horizontal");
        float z = Input.GetAxis("Vertical");

        var pos = new Vector3(x, 0f, z);
        transform.position += pos * Time.deltaTime * _speed;

        if (x == 0 && z == 0)
            return;
        
        if (_replicator.Factory is ReplicatedPlayerFactory factory)
        {
            factory.NotifySynchronize(this);
        }
            
        
    }
}
