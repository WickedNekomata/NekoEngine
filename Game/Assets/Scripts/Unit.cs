using System.Collections;
using JellyBitEngine;


public enum UNIT_TYPE : short
{
    NONE,
    ALITA,
    ENEMY
};

public class Unit : JellyScript
{
    // General Stats
    public int damage = 25;
    public float mov_speed = 12.0f;
    public UNIT_TYPE type = UNIT_TYPE.NONE;

    // Life
    private int current_life = 100;
    public int CurrentLife
    {
        get { return current_life; }
        set
        {
            current_life = value;
        }
    }
    private int max_life = 100;
    public int MaxLife { get { return max_life; } }

    // Components
    private NavMeshAgent agent = null;
    private Alita alita = null;

    // Start is called before the first frame update
    public override void Start()
    {
        switch (type)
        {
            case UNIT_TYPE.ALITA:
                {
                    agent = gameObject.GetComponent<NavMeshAgent>();

                    alita = gameObject.GetComponent<Alita>();

                    break;
                }
            case UNIT_TYPE.ENEMY:
                {
                    //agent = gameObject.GetComponent<NavMeshAgent>();

                    break;
                }
        }
    }

    public void MoveTo(Vector3 world_pos)
    {
        agent.SetDestination(world_pos);
    }

    public void Hit(int damage)
    {
        CurrentLife -= damage;

        if (current_life <= 0)
        {
            Destroy(gameObject);
            //if (type == UNIT_TYPE.ALITA)
            //    alita.LethalHit();
            //else
            //    Destroy(gameObject, 0.2f);
        }
    }

}
