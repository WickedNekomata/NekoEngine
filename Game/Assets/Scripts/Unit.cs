using System.Collections;
using JellyBitEngine;

public class Unit : JellyScript
{
    public enum UNIT_TYPE : short
    {
        NONE,
        ALITA,
        ENEMY
    };

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
    public int max_life = 100;
    public int MaxLife { get { return max_life; } }

    // Components
     private NavMeshAgent agent = null;

    // Start is called before the first frame update
    public override void Start()
    {
        switch (type)
        {
            case UNIT_TYPE.ALITA:
                {
                    agent = gameObject.GetComponent<NavMeshAgent>();
      
                    break;
                }
            case UNIT_TYPE.ENEMY:
                {
      
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
            Destroy(gameObject);

    }

}
