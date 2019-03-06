using System.Collections;
using JellyBitEngine;
using JellyBitEngine.SceneManager;

public class PlayerPrefsTest : JellyScript
{
    float life = 50f;
    string name = "Alita";
    bool dead = false;

    //Use this method for initialization
    public override void Awake()
    {
        life = (float)PlayerPrefs.GetNumber("life");
        name = PlayerPrefs.GetString("name");
        dead = PlayerPrefs.GetBoolean("dead");
    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();
        Debug.Log("Life: " + life.ToString());
        Debug.Log("Name: " + name);
        Debug.Log("Dead: " + dead.ToString());

        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            life = 100f;
            name = "Gally";
            dead = true;

            PlayerPrefs.SetNumber("life", life);
            PlayerPrefs.SetString("name", name);
            PlayerPrefs.SetBoolean("dead", dead);
            PlayerPrefs.Save();
        }

        if (Input.GetKeyDown(KeyCode.KEY_2))
        {
            PlayerPrefs.DeleteAll();
            PlayerPrefs.Save();
        }

        if(Input.GetKeyDown(KeyCode.KEY_0))
        {
            SceneManager.LoadScene("1");
        }
    }
}

