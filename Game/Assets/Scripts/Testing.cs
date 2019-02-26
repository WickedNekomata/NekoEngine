using System.Collections;
using JellyBitEngine;

public class Testing : JellyScript
{
    public float spot = 8f;
    public GameObject randomObject;
    public Transform randomTransform;

    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();
        Debug.LogError("randomObjects name is " + randomObject.name);
        Debug.LogError("randomTransform's name is " + randomTransform.gameObject.name + "and his position is " + randomTransform.position.ToString());
        

        gameObject.transform.position += Vector3.forward * Time.deltaTime * spot;
    }
}

