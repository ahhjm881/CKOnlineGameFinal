using System;
using System.Collections;
using System.Collections.Generic;
using CKPacket;
using UnityEngine;

public class ItemGiver : MonoBehaviour
{
    [SerializeField] private string playerName;
    [SerializeField] private string itemName;
    [SerializeField] private uint count;

    private void Start()
    {
        Client.TCP.AddPacketReceiver(header =>
        {
            var items = header.CreateMessage<resInventoryItems>();
            foreach (var item in items.Items)
            {
                Debug.Log($"name: {item.Name}\nitem name: {item.ItemName}\ncount: {item.Count}");
            }
        }, resInventoryItems.Descriptor.FullName);
    }

    public void AddToItem()
    {
        Client.TCP.SendPacket(new reqAddToItem()
        {
            Item = new itemInfo()
            {
                Count = count,
                Name = playerName,
                ItemName = itemName
            }
        });
    }

    public void RemoveFromItem()
    {
        Client.TCP.SendPacket(new reqRemoveFromItem()
        {
            Item = new itemInfo()
            {
                Count = 0,
                Name = playerName,
                ItemName = itemName
            }
        });
    }

    public void ReqInventoryItems()
    {
        Client.TCP.SendPacket(new reqInventoryItems()
        {
            Name = playerName
        });
    }
}
