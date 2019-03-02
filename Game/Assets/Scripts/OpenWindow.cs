using System.Collections;
using JellyBitEngine;

public class OpenWindow : JellyScript
{

    public GameObject window;
    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {

    }

    public void OnClick()
    {
        window.active = !window.active;
    }
}

