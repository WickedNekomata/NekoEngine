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
        Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
        RaycastHit hitInfo;
        if (Physics.Raycast(ray, out hitInfo, float.MaxValue, (uint)mask.masks, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            NavMeshAgent agent = gameObject.GetComponent<NavMeshAgent>();
            agent.SetDestination(hitInfo.point);
        }
    }
}

