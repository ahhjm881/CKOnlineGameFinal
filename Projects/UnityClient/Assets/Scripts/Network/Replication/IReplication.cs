using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface IReplication
{
    public Replicator Replicator { get; set; }
    public int Index { get; }
    public float R { get; }
    public float G { get; }
    public float B { get; }
    public float A { get; }
    public void OnInit();
    public void OnExpired();
}
