import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import missingno as ms
import seaborn as sns

from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from sklearn.metrics import accuracy_score
from sklearn import metrics

data = pd.read_csv("/home/someone/work/data/combine_data/combined_data.csv")
print(data.head())
print(data.shape)
print(data.info())
print("data.isnull: ", data.isnull().sum())
print(data.describe())
#ms.matrix(data)
print(data.corr())
dataheatmap = data.drop(["counter", "locked", "users"], axis=1)
sns.heatmap(dataheatmap.corr(), cmap="coolwarm")

#sns.swarmplot
#sns.swarmplot(x="bytes_counter", y="min_flt", data=data, palette="Set1")
print("dataheatmap shape:", dataheatmap.shape)

X = dataheatmap.drop("mal_beni", axis=1)
y = dataheatmap["mal_beni"]

plt.show(block=True)
    
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=.25)

clf = SVC(kernel="linear")
clf.fit(X_train, y_train)

y_pred = clf.predict(X_test)

print("-----accuracy-----")
print(accuracy_score(y_test, y_pred))

print("------y-test------")

print(y_test.mean())
print(metrics.confusion_matrix(y_test, y_pred))

print("-----precision------")
print(metrics.precision_score(y_test, y_pred))

print("-----recall-----")
print(metrics.recall_score(y_test, y_pred))

print("-----f1 score-----")
print(metrics.f1_score(y_test, y_pred))
