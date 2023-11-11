package cn.lanthing.activity.main

import android.util.Log
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.foundation.text.KeyboardOptions
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
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.toLowerCase
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import cn.lanthing.R
import cn.lanthing.ui.theme.AppTheme


// 用Jetpack Compose而不是View那套，是为了将来平稳过渡到Compose Multiplatform UI，进而支持iOS

@Composable
fun Logging() {
    AppTheme {
        Surface {
            Column(modifier = Modifier.fillMaxSize(),
                verticalArrangement = Arrangement.Center,
                horizontalAlignment = Alignment.CenterHorizontally) {
                LinearProgressIndicator(
                    modifier = Modifier.width(128.dp),
                )
                Text(
                    text = stringResource(id = R.string.logging),
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainPage(connect: (deviceID: Long, accessCode: String) -> Unit) {
    var deviceID by remember { mutableStateOf(0L) }
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
                    value = if (deviceID == 0L) "" else deviceID.toString(),
                    onValueChange = {
                        var it2 = it.filter { c -> c.isDigit() }
                        if (it2.length > 9) {
                            it2 = it2.substring(0, 9)
                        }
                        deviceID = if (it2.isEmpty()) 0L else it2.toLong()

                    },
                    label = { Text(stringResource(R.string.device_id)) },
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                )
                OutlinedTextField(
                    value = accessCode,
                    onValueChange = {
                        Log.d("MainUI", "it is $it")
                        // isLetterOrDigit会把汉字也包含进来
                        var it2 = it.filter { c -> c.isDigit() || c.isUpperCase() || c.isLowerCase() }.lowercase()
                        Log.d("MainUI", "it2 is $it2")
                        if (it2.length > 6) {
                            it2 = it2.substring(0, 6)
                        }
                        accessCode = it2
                    },
                    label = { Text(stringResource(R.string.access_code)) })
                Button(
                    onClick = { connect(deviceID, accessCode) },
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

@Composable
fun Connecting() {
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

@Composable
fun ErrorMessage(errCode: Int, back: () -> Unit) {
    AppTheme {
        Surface {
            Column {
                Text(text = "ErrorCode $errCode")
                Button(onClick = back) {
                    Text(text = "Back")
                }
            }
        }
    }
}

@Preview(showBackground = true, showSystemUi = true)
@Composable
fun MainPagePreview() {
    MainPage { _, _ ->
        {
        }
    }
}

@Preview(showBackground = true, showSystemUi = true)
@Composable
fun LoggingPreview() {
    Logging()
}