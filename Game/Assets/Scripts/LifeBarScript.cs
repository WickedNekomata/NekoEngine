using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class LifeBarScript : JellyScript
{
    private AreaAttk player = null;
    public GameObject alita;
    private RectTransform rect = null;
    private float maxHP;
    private uint maxSize;
    //Use this method for initialization
    public override void Awake()
    {
        //player = alita.GetComponent<AreaAttk>();
        //rect = gameObject.GetComponent<RectTransform>();
        //maxHP = player.life;
    }

    //Called every frame
    public override void Update()
    {
        
        if(player == null)
        {
            player = alita.GetComponent<AreaAttk>();
            maxHP = player.life;
        }

        if(rect == null)
        {
            rect = gameObject.GetComponent<RectTransform>();
            maxSize = rect.x_dist;
        }        

        rect.x_dist = (uint)((maxSize * player.life) / maxHP);
    }
}

