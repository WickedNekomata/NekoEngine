using System.Collections;
using JellyBitEngine;

public class Testing : JellyScript
{
    public float spot = 8f;
    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {
        gameObject.transform.position += Vector3.forward * Time.deltaTime * spot;
    }
}

