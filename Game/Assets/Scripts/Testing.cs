using System.Collections;
using JellyBitEngine;

public class Testing : JellyScript
{
    public LayerMask layerMask = new LayerMask();
    public float spot = 8f;
    public GameObject anotherGO;

    Testing testing;

    //Use this method for initialization
    public override void Awake()
    {
        testing = gameObject.GetComponent<Testing>();
    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_1))
            testing.SetActive(false);

        Debug.ClearConsole();
        Debug.Log("Hey dude!");
        //Debug.LogError("randomObjects name is " + randomObject != null ? randomObject.name : "null");
        //Debug.LogError("randomTransform's name is " + randomTransform.gameObject.name + "and his position is " + randomTransform.position.ToString());
        
        //if(Input.GetKeyDown(KeyCode.KEY_1))
        //{
        //    Debug.ClearConsole();
        //    Testing testing = gameObject.GetComponent<Testing>();
        //    Debug.Log("Spot is " + testing.spot.ToString());

        //    DataHolder holder = anotherGO.GetComponent<DataHolder>();
        //    if(holder != null)
        //    {
        //        Debug.Log("JODER ESTO ES BRUTAL");
        //        Debug.Log("Holders a is " + holder.a.ToString());
        //        Debug.Log("Holders b is " + holder.b);                
        //    }         

        //    //OverlapHit[] hitInfo;
        //    //if (Physics.OverlapSphere(100f, transform.position, out hitInfo, (uint)layerMask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        //    //{
               
        //    //}
        //    //Debug.LogError("Im destroying the copy " + tempGO.name);
        //    //Destroy(tempGO);
        //}

        //gameObject.transform.position += Vector3.forward * Time.deltaTime * spot;
    }
}

