using System.Collections;
using JellyBitEngine;
using System;

public class Testing : JellyScript
{
    public float speed = 0f;
    
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        transform.position += Vector3.forward * speed * Time.deltaTime;
        transform.rotation *= Quaternion.Rotate(Vector3.up, speed * Time.deltaTime * 20f);
       
    }
}

