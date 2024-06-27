using System;
using System.Collections;
using System.Collections.Generic;
using CKPacket;
using UnityEngine;
using TMPro;
using UnityEngine.UI;

public class Shop : MonoBehaviour
{
    [SerializeField] private Transform _content;
    
    private TMP_Text[] _texts;
    private List<shopItem> _infos = new();

    public List<shopItem> Items => _infos;

    private void Awake()
    {
        _texts = new TMP_Text[_content.childCount];
        for (int i = 0; i < _content.childCount; i++)
        {
            _texts[i] = _content.GetChild(i).GetComponentInChildren<TMP_Text>();
        }
    }

    private void Start()
    {
        
        Client.TCP.AddPacketReceiver(header =>
        {
            var items = header.CreateMessage<resShopItemList>();
            _infos.Clear();
            foreach (var item in items.Items)
            {
                _infos.Add(item);
            }

            OnViewUpdateShopItem();
        }, resShopItemList.Descriptor.FullName);
        
        Client.TCP.SendPacket(new reqShopItemList());
    }
    
    private void OnViewUpdateShopItem()
    {
        for (int i = 0; i < Mathf.Min(_texts.Length, _infos.Count); i++)
        {
            var text = _texts[i];
            var info = _infos[i];
            var btn = text.GetComponentInParent<Button>().onClick;
            btn.RemoveAllListeners();
            btn.AddListener(() =>
            {
                Client.TCP.SendPacket(new reqShopBuy()
                {
                    Count = 1,
                    PlayerName = Client.Name,
                    ItemID = info.ItemId
                });
            });

            text.text = $"{info.ItemName}\n{info.ItemDesc}\n{info.ItemCount}";
        }
    }
}
