using System.Collections;
using JellyBitEngine;

public class MoveToClick : JellyScript
{
    public LayerMask layerMask = new LayerMask();

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
        {
            NavMeshAgent agent = gameObject.GetComponent<NavMeshAgent>();
            if(agent != null)
            {
                Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);

                RaycastHit hitInfo;

                if(Physics.Raycast(ray, out hitInfo, float.MaxValue, (uint)layerMask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
                {
                    Debug.Log("I set up the destination");
                    agent.SetDestination(hitInfo.point);
                }
            }
            else           
                Debug.LogError("I have not encountered a NavMeshAgent component");            
        }
    }
}

