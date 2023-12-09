using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class NetworkInitializer : MonoBehaviour
{
    // ScriptExecutionOrder가 monoBehaviour 중 최상위로 배치되어 있음
    private void Awake()
    {
        Client.Start();
    }

    private void OnDestroy()
    {
        Client.Close();
    }
}
