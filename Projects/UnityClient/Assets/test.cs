using System;
using System.Collections;
using System.Collections.Generic;
using CKPacket;
using UnityEngine;

public class test : MonoBehaviour
{
    [SerializeField] private int iter = 100;

    private void Update()
    {
        for (int i = 0; i < iter; i++)
        {
            Client.TCP.SendPacket(new CKPacket.reqPing() {});
        }
    }
}
