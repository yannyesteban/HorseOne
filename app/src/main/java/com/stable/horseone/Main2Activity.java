package com.stable.horseone;

import android.content.Intent;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class Main2Activity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);

        Button boton = findViewById(R.id.button);
        boton.setText("Real Native");

        boton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent Song_detail = new Intent(getApplicationContext(), android.app.NativeActivity.class);
                startActivity(Song_detail);
            }
        });


        Button boton2 = findViewById(R.id.button2);
        boton2.setText("Sub Clase");

        boton2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent Song_detail = new Intent(getApplicationContext(), Main3Activity.class);
                startActivity(Song_detail);
            }
        });


    }
}
