using System.Collections;
using JellyBitEngine;

public class Testing : JellyScript
{
    public LayerMask layerMask = new LayerMask();
    public float spot = 8f;
    public GameObject prefab;

    //Use this method for initialization
    public override void Awake()
    {
        
    }

    //Called every frame
    public override void Update()
    {

        //Debug.LogError("randomObjects name is " + randomObject != null ? randomObject.name : "null");
        //Debug.LogError("randomTransform's name is " + randomTransform.gameObject.name + "and his position is " + randomTransform.position.ToString());
        
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            OverlapHit[] hitInfo;
            if (Physics.OverlapSphere(100f, transform.position, out hitInfo, (uint)layerMask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                Debug.ClearConsole();
                Debug.Log("I hitted " + hitInfo.Length.ToString() + " objects");

                foreach(OverlapHit hit in hitInfo)
                {
                    Debug.Log("I hitted " + hit.gameObject.name);
                }
            }
            //Debug.LogError("Im destroying the copy " + tempGO.name);
            //Destroy(tempGO);
        }

        //gameObject.transform.position += Vector3.forward * Time.deltaTime * spot;
    }
}

