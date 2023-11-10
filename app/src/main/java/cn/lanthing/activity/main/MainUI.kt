package cn.lanthing.activity.main

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
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import cn.lanthing.R
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
                    value = " ",
                    onValueChange = { deviceID = it.toLong() },
                    label = { Text(stringResource(R.string.device_id)) })
                OutlinedTextField(
                    value = " ",
                    onValueChange = { accessCode = it },
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
    MainPage { _, _ -> {
    }}
}

@Preview(showBackground = true, showSystemUi = true)
@Composable
fun LoggingPreview() {
    Logging()
}