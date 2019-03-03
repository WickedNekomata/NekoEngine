using System.Collections;
using JellyBitEngine;
using System;


class NetmanBehaviour : JellyScript
{
    public GameObject alita;
    public GameObject particleShoot;
    //For netman shoot
    public GameObject bullet;
    public GameObject instanciatePosition; //bullet instanciate position.
    public float minDistance = 5.0f; //min_distance to shoot.
    public float timeToShot = 4.0f; // time between shoots.
    public float timeToLoad = 3.0f; // time between shoots.
    float actualTime = 0.0f; // count actual time.

    //For netman rotation
    public float min_angle = 0.01f; //min angle to stop rotation.
    double Rad2Deg = 57.29577;

    private bool startParticles = true;

    public override void Awake()
    {
        BulletMovement.direction = alita;
    }

    public override void Update()
    {
        Vector3 distance = this.transform.position - alita.transform.position; //distance between alita to netman
        double distanceMagnitude = distance.magnitude; // magnitude

        if (distanceMagnitude < (minDistance * minDistance)) // calculate distance (min_distance * min_distance) because we don't have method: sqrtmagnitude. 
        {
            // If min alita is in min_distance shoot or prepare shoot
            if (actualTime >= timeToLoad && startParticles)
            {
                // Play charge particles
                particleShoot.GetComponent<ParticleEmitter>().Play();
                actualTime += Time.deltaTime;

                // Play charge sound
                //gameObject.GetComponent<SoundEmitter?>().Play(chargeFX?);

                // Avoid entering here again
                startParticles = false;
            }
            else if (actualTime >= timeToShot)
            {
                // Stop partilce effect
                particleShoot.GetComponent<ParticleEmitter>().Stop();
                startParticles = true; // Allow particles to start again later

                // Play shoot sound
                //gameObject.GetComponent<SoundEmitter?>().Play(shootFX?);

                // Create the bullet itself
                GameObject newBullet = GameObject.Instantiate(bullet, instanciatePosition.transform.position);

                // Reset shoot timer
                actualTime = 0.0f;
            }
            else
            {
                // Update shoot time
                actualTime += Time.deltaTime;
            }

            //Kinematic face movement for netman
            Vector3 faceDirection = (alita.transform.position - transform.position).normalized();
            double targetDegrees = Math.Atan2(faceDirection.x, faceDirection.z) * Rad2Deg;

            transform.rotation = Quaternion.Rotate(Vector3.up, (float)(-90.0f +targetDegrees));
        }
    }
}
