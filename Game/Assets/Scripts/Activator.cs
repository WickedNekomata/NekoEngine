using System.Collections;
using JellyBitEngine;
using JellyBitEngine.UI;

public class Activator : JellyScript
{
    public GameObject cube;

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            cube.active = !cube.active;
        }
    }
}

