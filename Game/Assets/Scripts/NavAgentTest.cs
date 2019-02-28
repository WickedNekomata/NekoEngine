using System.Collections;
using JellyBitEngine;

public class NavAgentTest : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            NavMeshAgent agent = gameObject.GetComponent<NavMeshAgent>();
            if(agent != null)
            {
                Debug.Log("I found an agent, SetDestination to 0,0,0");
                agent.SetDestination(new Vector3(0, 0, 0));
            }
            else
            {
                Debug.Log("I have not found an agent :c");
            }
        }
    }
}

