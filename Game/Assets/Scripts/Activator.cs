using System.Collections;
using JellyBitEngine;

public class Activator : JellyScript
{
    public GameObject temp;
    public int cavaio = 2;

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            temp.active = !temp.active;
        }
    }
}

