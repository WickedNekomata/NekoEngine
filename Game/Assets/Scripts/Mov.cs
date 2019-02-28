using System.Collections;
using JellyBitEngine;

public class Mov : JellyScript
{
    private RaycastHit hit;
    private int mask = LayerMask.GetMask("Default");
    private NavMeshAgent agent = null;

    public override void Start()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
    }

    //Called every frame
    public override void Update()
    {
        CheckForMouseClick();
    }

    private void CheckForMouseClick()
    {
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_RIGHT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if (Physics.Raycast(ray, out hit, float.MaxValue, (uint)mask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                if (agent == null)
                    agent = gameObject.GetComponent<NavMeshAgent>();

                if (agent != null)
                {
                    Debug.Log("GOING TO SPOT");
                    agent.SetDestination(hit.point);
                }
                else
                    Debug.Log("AGENT IS NULL");
            }
        }
    }

}

