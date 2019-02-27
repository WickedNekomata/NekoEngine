using System.Collections;
using JellyBitEngine;

public class Instance : JellyScript
{
    float initTime = 0f;

    //Use this method for initialization
    public override void Awake()
    {
        Debug.Log(gameObject.name + " awaked");
        initTime = Time.time;
    }

    //Called every frame
    public override void Update()
    {
        if (Time.time > initTime + 5)
            Destroy(gameObject);
    }
}

