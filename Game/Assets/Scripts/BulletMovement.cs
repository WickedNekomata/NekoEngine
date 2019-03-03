using System.Collections;
using JellyBitEngine;


class BulletMovement : JellyScript
{
    public GameObject direction;
    public float speed = 25f;
    public float life = 10f;
    public bool isInmortal = true;

    private float lifeTime = 0f;
    private Vector3 dir = new Vector3(0, 0, 0);
    private bool getDirection = true;

    //Life and dmg
    public int damage = 10;
    private int current_life = 100;

    public override void Awake()
    {
        lifeTime = Time.time;
    }

    public override void Update()
    {
        if (getDirection)
        {
            dir = (direction.transform.position - transform.position).normalized();

            getDirection = false;
        }

        transform.position += dir * (speed * Time.deltaTime);

        if (Time.time > lifeTime + life)
        {
            if (!isInmortal)
                Destroy(gameObject);
        }
    }

    public void Hit(int damage)
    {
        current_life -= damage;

        if (current_life <= 0)
            if (!isInmortal)
                Destroy(gameObject);
    }

}