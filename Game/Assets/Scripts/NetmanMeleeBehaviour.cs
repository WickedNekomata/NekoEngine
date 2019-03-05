using System.Collections;
using JellyBitEngine;
using System;



class NetmanMeleeBehaviour : JellyScript
{
    //Alita dependences
    public GameObject alita;
    Unit alita_unit = null;

    //Netman Stats
    int life = 50;
    int damage = 20;

    //Distance to move and offset
    public float minDistance = 0.5f;
    public float offset = 1.0f;

    //Attack variables
    public float attack_dist = 2.0f;
    public float attk_period = 1.0f;
    private float attk_cool_down = 0.0f;

    //Agent
    private NavMeshAgent agent = null;


    private enum Enemy_State
    {
        IDLE,
        GOING_TO_ATTK,
        ATTK,
    }
    Enemy_State state = Enemy_State.IDLE;

    //For netman rotation
    public float min_angle = 0.01f; //min angle to stop rotation.
    double Rad2Deg = 57.29577;

    public override void Awake()
    {
        agent = gameObject.GetComponent<NavMeshAgent>();
        alita_unit = alita.GetComponent<Unit>();
    }

    public override void Update()
    {
        Vector3 distance = this.transform.position - alita.transform.position; //distance between alita to netman
        double distanceMagnitude = distance.magnitude; // magnitude

        switch (state)
        {
            case Enemy_State.IDLE:
                if (distanceMagnitude < (minDistance * minDistance))
                    state = Enemy_State.GOING_TO_ATTK;
                break;

            case Enemy_State.GOING_TO_ATTK:
                if (agent == null)
                    agent = gameObject.GetComponent<NavMeshAgent>();
                agent.SetDestination(alita.transform.position);

                float diff = (float)(alita.transform.position - transform.position).magnitude;
                if (diff <= offset)
                {
                    agent.SetDestination(transform.position);
                    state = Enemy_State.ATTK;
                }

                if (distanceMagnitude < (minDistance * minDistance))
                    state = Enemy_State.IDLE;
                break;

            case Enemy_State.ATTK:
                if(alita != null)
                {
                    attk_cool_down += Time.deltaTime;

                    //Attack every second
                    if (attk_cool_down >= attk_period)
                    {
                        if(alita_unit == null && alita != null)
                            alita_unit = alita.GetComponent<Unit>();

                        if (alita_unit != null)
                            alita_unit.Hit(damage);

                        attk_cool_down = 0.0f;
                    }
                }
                break;
                
        }

        if (state != Enemy_State.IDLE)
        {
            //Kinematic face movement for netman
            Vector3 faceDirection = (alita.transform.position - transform.position).normalized();
            double targetDegrees = Math.Atan2(faceDirection.x, faceDirection.z) * Rad2Deg;
            transform.rotation = Quaternion.Rotate(Vector3.up, (float)(-90.0f + targetDegrees));
        }
    }
}
