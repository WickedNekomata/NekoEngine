using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class LifeBarScript : JellyScript
{
    private AreaAttk player = null;
    public GameObject alita;
    private RectTransform rect = null;
    private float maxSize;
    //Use this method for initialization
    public override void Awake()
    {
        player = alita.GetComponent<AreaAttk>();
        rect = gameObject.GetComponent<RectTransform>();
        maxSize = rect.x_dist;
    }

    //Called every frame
    public override void Update()
    {
        if(player == null)
        {
            player = alita.GetComponent<AreaAttk>();
        }
        if(rect == null)
        {
            rect = gameObject.GetComponent<RectTransform>();
        }

        rect.x_dist = (uint)((rect.x_dist * 100) / player.life);
    }
}

