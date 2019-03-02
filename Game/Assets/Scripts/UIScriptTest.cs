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
        rect = gameObject.GetComponent<RectTransform>();
        btn = gameObject.GetComponent<Button>();
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
                case ButtonState.R_CLICK:
                    break;
                case ButtonState.L_CLICK:
                    break;
            }
        }

        //RectTransform - Set and Get
        //rect.x
        //rect.y
        //rect.x_dist
        //rect.y_dist
    }

    //Set trhough on inspector on component buton, like unity (give gameobject where is the script and select script and method)
    public void OnClick()
    {
        Debug.Log("GG");
    }
}

