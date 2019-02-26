using System.Collections;
using JellyBitEngine;

public class RayCaster : JellyScript
{
    public LayerMask mask = new LayerMask();
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            RaycastHit hitInfo;
            if (Physics.Raycast(ray, out hitInfo, float.MaxValue, (uint)mask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                if(hitInfo != null)
                {
                    Debug.Log("I raycasted at" + hitInfo.point.ToString());
                }
                else
                {
                    Debug.LogError("Hey dude hitinfo is null");
                }             
           
                //Debug.LogWarning("I hitted " + hitInfo.gameObject.name + " at the point " + hitInfo.point.ToString());
                //NavMeshAgent agent = gameObject.GetComponent<NavMeshAgent>();
                //if(agent != null)
                //{
                //    Debug.LogWarning("I founded a NavMeshAgent component");
                //    agent.SetDestination(hitInfo.point);
                //}            
            }
            NavMeshAgent agent = gameObject.GetComponent<NavMeshAgent>();
            if(agent != null)
            {
                Debug.LogWarning("I founded a NavMeshAgent component");
            }
            else
            {
                Debug.Log("Hey dude where is my agent");
            }
        }  
    }
}

