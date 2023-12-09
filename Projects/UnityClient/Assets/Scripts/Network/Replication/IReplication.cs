using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public interface IReplication
{
    public int Index { get; }
    public void OnInit();
    public void OnExpired();
}
