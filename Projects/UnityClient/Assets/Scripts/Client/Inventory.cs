using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using TMPro;
using UnityEngine;
using CKPacket;

public class Inventory : MonoBehaviour
{
    [SerializeField] private Transform _content;
    private TMP_Text[] _slots;
    private List<itemInfo> _infos;

    private Shop _shop;

    private void Awake()
    {
        _slots = new TMP_Text[_content.childCount];
        for (int i = 0; i < _content.childCount; i++)
        {
            _slots[i] = _content.GetChild(i).GetComponentInChildren<TMP_Text>();
        }
        
        _shop = GameObject.Find("Shop").GetComponent<Shop>();
        _infos = new();

        Client.TCP.AddPacketReceiver(OnUpdateInventory, resInventoryItems.Descriptor.FullName);
    }

    private void OnUpdateInventory(PackedHeader header)
    {
        var packet = header.CreateMessage<resInventoryItems>();

        _infos.Clear();
        print(packet.Items.Count);
        foreach (itemInfo packetItem in packet.Items)
        {
            _infos.Add(packetItem);
        }
        
        UpdateView();
    }

    public void UpdateView()
    {
        print(_slots.Length);
        print(_infos.Count);
        
        _infos.Sort((x, y) =>
        {
            if (x.Itemid > y.Itemid) return 1;
            if (x.Itemid < y.Itemid) return 0;
            else return 0;
        });
        
        for (int i = 0; i <Mathf.Min(_infos.Count, _slots.Length); i++)
        {
            print(_infos);
            _slots[i].text = $"{_infos[i].ItemName}\n: {_infos[i].Count}";
        }
    }
 }
