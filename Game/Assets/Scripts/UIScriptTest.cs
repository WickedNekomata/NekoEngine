using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class UIScriptTest : JellyScript
{
    private Button btn;
    private RectTransform rect;

    //Use this method for initialization
    public override void Awake()
    {
        btn = gameObject.GetComponent<Button>();
        rect = gameObject.GetComponent<RectTransform>();
    }

    //Called every frame
    public override void Update()
    {
        if (UI.UIHovered())
        {
            switch (btn.state)
            {
                case ButtonState.IDLE:
                    break;
                case ButtonState.HOVERED:
                    break;
                case ButtonState.L_CLICK:
                    break;
                case ButtonState.R_CLICK:
                    break;
            }
        }

        //RectTransform, get and set
        //rect.x
        //rect.y
        //rect.x_dist
        //rect.y_dist
    }

    //Set through the inpector at BUTTON COMPONENT
    public void OnClick()
    {
        Debug.Log("GG");
    }
}

