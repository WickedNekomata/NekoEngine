using System.Collections;
using FlanEngine;

public class test : FlanScript
{
    public float speed = 1f;

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();
        Debug.Log("Game DT: " + Time.deltaTime);
        Debug.Log("Real DT: " + Time.realDeltaTime);
        Debug.Log("Game Time: " + Time.time);
        Debug.Log("Real Time: " + Time.realTime);

        transform.position += Vector3.forward * Time.deltaTime * speed;
        transform.rotation = transform.rotation.Rotate(Vector3.up, speed);     
    }
}

