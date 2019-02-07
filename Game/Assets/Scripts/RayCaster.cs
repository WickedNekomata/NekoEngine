using System.Collections;
using JellyBitEngine;

public class RayCaster : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();

        //Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        //Debug.Log("Ray fields: \nPosition:" + ray.position.ToString() + " Direction: " + ray.direction.ToString());

        Debug.Log("The bitset for the Layer \"Default\" is " + LayerMask.GetMask("Default"));
        Debug.Log("The bitset for the unexistent layer is " + LayerMask.GetMask("i dont exist"));
    }
}

