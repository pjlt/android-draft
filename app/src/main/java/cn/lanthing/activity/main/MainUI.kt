package cn.lanthing.activity.main

import android.content.Intent
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import cn.lanthing.R
import cn.lanthing.activity.stream.Stream
import cn.lanthing.ui.theme.AppTheme

@Composable
fun Logging() {
    AppTheme {
        Surface {
            Column {
                LinearProgressIndicator(
                    modifier = Modifier.width(64.dp),
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainPage() {
    val context = LocalContext.current
    var deviceID by remember { mutableStateOf("") }
    var accessCode by remember { mutableStateOf("") }
    AppTheme {
        Surface(
            modifier = Modifier
                .padding(24.dp)
                .fillMaxSize()
                .wrapContentSize(Alignment.Center)
        ) {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.padding(24.dp)
            ) {
                OutlinedTextField(
                    value = " ",
                    onValueChange = { deviceID = it },
                    label = { Text(stringResource(R.string.device_id)) })
                OutlinedTextField(
                    value = " ",
                    onValueChange = { accessCode = it },
                    label = { Text(stringResource(R.string.access_code)) })
                Button(
                    onClick = { context.startActivity(Intent(context, Stream::class.java)) },
                    modifier = Modifier
                        .padding(8.dp)
                        .fillMaxWidth()
                ) {
                    Text(text = "Link")
                }
            }
        }
    }
}

@Preview(showBackground = true, showSystemUi = true)
@Composable
fun MainPagePreview() {
    MainPage()
}

@Preview(showBackground = true, showSystemUi = true)
@Composable
fun LoggingPreview() {
    Logging()
}