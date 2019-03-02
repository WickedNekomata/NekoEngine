using System.Collections;
using JellyBitEngine;

public class MOTOR : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            gameObject.transform.position += Vector3.forward * Time.deltaTime * 5f;
        }
    }
}

