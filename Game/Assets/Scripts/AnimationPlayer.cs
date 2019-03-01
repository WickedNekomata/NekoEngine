using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class AnimationPlayer : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_1))
        {
            RectTransform rect = gameObject.GetComponent<RectTransform>();
            if(rect != null)
            {
                rect.x = 0;
                rect.y = 0;
                rect.x_dist = 0;
                rect.y_dist = 0;
            }
        }
    }
}

