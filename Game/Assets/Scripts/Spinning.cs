using System.Collections;
using JellyBitEngine;

public class Spinning : JellyScript
{
    public float moveSpeed = 10f;
    public float rotSpeed = 30f;

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();
        Debug.Log("Im spinning away");

        transform.position += Vector3.forward * moveSpeed * Time.deltaTime;
        transform.rotation *= Quaternion.Rotate(Vector3.up, rotSpeed * Time.deltaTime);
    }
}

