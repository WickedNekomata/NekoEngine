using System.Collections;
using JellyBitEngine;

public class Testing : JellyScript
{
    public float spot = 8f;
    public GameObject randomObject;

    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();

        if (randomObject != null)
            Debug.Log("randomObject is not null");
        else
            Debug.Log("randomObject is null");

        //Debug.LogError("randomObjects name is " + randomObject != null ? randomObject.name : "null");
        //Debug.LogError("randomTransform's name is " + randomTransform.gameObject.name + "and his position is " + randomTransform.position.ToString());
        
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            //GameObject tempGO = GameObject.Instantiate(randomObject);
            //Debug.LogError("Im destroying the copy " + tempGO.name);
            //Destroy(tempGO);
        }

        //gameObject.transform.position += Vector3.forward * Time.deltaTime * spot;
    }
}

