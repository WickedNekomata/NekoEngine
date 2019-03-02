using System.Collections;
using JellyBitEngine;

public class Activator : JellyScript
{
    public GameObject temp;

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            gameObject.active = !gameObject.active;
        }
    }
}

