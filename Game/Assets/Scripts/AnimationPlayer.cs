using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class AnimationPlayer : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {
        Button button = gameObject.GetComponent<Button>();
        if (button != null)
        {
            button.SetKey(KeyCode.KEY_0);
            Debug.Log(button.state.ToString());
        }
    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_1))
        {
            Button button = gameObject.GetComponent<Button>();
            if(button != null)
            {
                button.SetKey(KeyCode.KEY_0);
                Debug.Log(button.state.ToString());
            }
        }
    }

    public void OnClick()
    {
        Debug.Log("Daddy I'm being clicked ;)");
    }
}

